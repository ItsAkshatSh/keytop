#define DT_DRV_COMPAT ti_tps65201a

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(tps65201a, CONFIG_INPUT_LOG_LEVEL);

#define REG_PRODUCT_ID 0x00
#define REG_FW_VERSION 0X01
#define REG_STATUS 0x02
#define REG_FINGER_COUNT 0x03
#define REG_TOUCH_DATA 0x04
#define REG_CONTACT_SIZE 0x05
#define REG_GESTURE 0x06
#define REG_CONTROL 0x07


/* gesture */
#define GESTURE_NONE 0x00
#define GESTURE_SCROLL_UP 0x01
#define GESTURE_SCROLL_DOWN 0x02
#define GESTURE_SCROLL_LEFT 0x03
#define GESTURE_SCROLL_RIGHT 0x04
#define GESTURE_3F_LEFT 0x05
#define GESTURE_3F_RIGHT 0x06
#define GESTURE_3F_UP 0x07
#define GESTURE_3F_DOWN 0x08
#define GESTURE_PINCH_IN 0x09
#define GESTURE_PINCH_OUT 0x0A
#define GESTURE_CODE_MAX 0x0A

#define TRACKPAD_MAX_X 4095
#define TRACKPAD_MAX_Y 4095

#define TAP_MAX_MS 150
#define TAP_MAX_DISTANCE 150

#define DOUBLE_TAP_MS 300

#define CLICK_ZONE_RATIO 50
#define PALM_SIZE_THRESHOLD 180

#define MOVEMENT_DIVISIOR 4

#define ACCEL_FAST_THRESHOLD 1000
#define ACCEL_FAST_MULTIPLIER 2

#define JITTER_THRESHOLD 3
#define GESTURE_COOLDOWN_MS 80

#define MIN_EVENT_INTERVAL_MS 8

#define I2C_MAX_RETRIES 3

#define SCROLL_STEP 3

#define RIGHT_ZONE_MIN ((TRACKPAD_MAX_X * CLICK_ZONE_RATIO) / 100)

enum touch_state {
    STATE_IDLE,
    STATE_TOUCHING,
    STATE_DRAGGING,
    STATE_TAP_PENDING
};

/*driver struct*/

struct tps65201a_data {
    const struct device *dev;
    struct gpio_callback rdy_cb;
    struct k_work  work;
    struct k_work_delayable tap_confirm_work;
    struct k_work_delayable gesture_cooldown_work;

    enum touch_state state;

    int16_t last_x;
    int16_t last_y;

    int16_t tap_start_x;
    int16_t tap_start_y;
    int64_t touch_start_ms;

    int64_t last_tap_ms;
    uint32_t last_tap_btn;
    uint32_t pending_btn;

    bool two_finger_was_scrolling;
    int64_t two_finger_start_ms;

    int64_t gesture_last_fired_ms[11];

    int64_t last_event_ms;
};

struct tps65201a_config {
    struct i2c_dt_spec i2c;
    struct gpio_dt_spec rdy_gpio;
};

/*I2C read-write*/

static int tps_read(const struct device *dev, uint8_t reg, uint8_t *buf, size_t len) {
    const struct tps65201a_config *cfg = dev->config ;
    int ret;

    for (int attempt = 0; attempt < I2C_MAX_RETRIES; attempt++){
        ret = i2c_write_read_dt(&cfg->i2c, reg, buf, len);
        if (ret == 0) {
            return 0;
        }
        LOG_WRN("I2C read reg=0x%02X failed (attempt %d/%d): %d", reg, attempt + 1, I2C_MAX_RETRIES, ret);
        k_sleep(K_USEC(10));
    }
    LOG_ERR("I2C read reg=0x%02X failed after %d attempts: %d", reg, I2C_MAX_RETRIES);
    return ret;
};

static int tps_write_byte(const struct device *dev, uint8_t reg, uint8_t value) {
    const struct tps65201a_config *cfg = dev->config ;
    uint8_t buf[2] = {reg, value};
    int ret;

    for (int attempt = 0; attempt < I2C_MAX_RETRIES; attempt++){
        ret = i2c_write_dt(&cfg->i2c, buf, sizeof(buf));
        if (ret == 0) {
            return 0;
        }
        LOG_WRN("I2C write reg=0x%02X failed (attempt %d/%d): %d", reg, attempt + 1, I2C_MAX_RETRIES, ret);
        k_sleep(K_USEC(10));
    }
    LOG_ERR("I2C write reg=0x%02X failed after %d attempts: %d", reg, I2C_MAX_RETRIES, ret);
    return ret;
};

/*touch states reset*/

static void reset_touch_state(struct tps65201a_data *data) {
    data->state = STATE_IDLE;
    data->last_x = 0;
    data->last_y = 0;
    data->touch_start_ms = 0;
    data->two_finger_start_ms = 0;
    data->two_finger_was_scrolling = false;


    k_work_cancel_delayable(&data->gesture_cooldown_work);
    LOG_DBG("Touch state reset");
}

/*gesture timeout handler - if touch is too long*/

static void gesture_timeout_handler(struct k_work *work)  {
    struct k_work_delayable *dw =  k_work_delayable_from_work(work);
    struct tps65201a_data *data = CONTAINER_OF(dw, struct tps65201a_data, gesture_cooldown_work);

    if(data->state != STATE_IDLE){
        LOG_DBG("Gesture timeout expired, resetting touch state");
        reset_touch_state(data);

        k_work_cancel_delayable(&data->tap_confirm_work);
    }
}

/*Click Zones*/

static uint32_t get_click_button(int16_t tap_start_x) {
    if (tap_start_x >= RIGHT_ZONE_MIN) {
        return INPUT_BTN_RIGHT;
    } else {
        return INPUT_BTN_LEFT;
    }
}

/*click emits*/

static void  emit_click(const struct device *dev, uint32_t btn) {
    input_report_key(dev, btn, 1, false, K_NO_WAIT);
    input_report_key(dev, btn, 0, true, K_NO_WAIT);
    LOG_INF("Click: %s", btn == INPUT_BTN_LEFT ? "LEFT" : "RIGHT");
}

static void emit_double_click(const struct device *dev, uint32_t btn) {
    emit_click(dev, btn);
    k_sleep(K_MSEC(50));
    emit_click(dev, btn);
    LOG_INF("Double Click: %s", btn == INPUT_BTN_LEFT ? "LEFT" : "RIGHT");
}

/* accel on trackpad */

static int16_t apply_acceleration(int16_t raw_delta) {
    if (raw_delta == 0){
        return 0;
    }
    int16_t divided = raw_delta / MOVEMENT_DIVISIOR;
    int16_t magnitude = abs(divided);

    return (magnitude >= ACCEL_FAST_THRESHOLD) ? divided * ACCEL_FAST_MULTIPLIER : divided;
}

/* Gesture cooldown */
static bool gesture_allowed(struct tps65201a_data *data, uint8_t gesture) {
    if (gesture == GESTURE_NONE || gesture > GESTURE_CODE_MAX) {
        return false;
    }

    int64_t now = k_uptime_get();
    int64_t elapsed = now - data->gesture_last_fired_ms[gesture];

    if (elapsed >= GESTURE_COOLDOWN_MS){
        data->gesture_last_fired_ms[gesture] = now;
        return true;
    }
    LOG_DBG("Gesture %d on cooldown (elapsed %lld ms)", gesture, elapsed, GESTURE_COOLDOWN_MS);
    return false;
}

/*Delayed Tap*/

static void tap_confirm_handler(struct k_work *work){
    struct k_work_delayable *dw = k_work_delayable_from_work(work);
    struct tps65201a_data *data = CONTAINER_OF(dw, struct tps65201a_data, tap_confirm_work);

    if (data->state == STATE_TAP_PENDING){
        emit_click(data->dev,data->pending_btn);
        data->state = STATE_IDLE;
        LOG_DBG("Single tap confirmed (no double tap)");
    }
}


/* two finger handler
scroll wheel --> positive --> natural scroll, negative --> reverse scroll
*/

static void handle_two_finger(const struct device *dev, struct tps65201a_data *data, uint8_t gesture)
{
    if (gesture == GESTURE_NONE || !gesture_allowed(data, gesture)){
        return;
    }

    switch (gesture){
        case GESTURE_SCROLL_UP:
            input_report_rel(dev, INPUT_REL_WHEEL, SCROLL_STEP, true, K_NO_WAIT);
            data->two_finger_was_scrolling = true;
            LOG_DBG("2F scroll UP");
            break;

        case GESTURE_SCROLL_DOWN:
            input_report_rel(dev, INPUT_REL_WHEEL, -SCROLL_STEP, true, K_NO_WAIT);
            data->two_finger_was_scrolling = true;
            LOG_DBG("2F scroll DOWN");
            break;

        case GESTURE_SCROLL_LEFT:
            input_report_re(dev, INPUT_REL_HWHEEL, -SCROLL_STEP, true, K_NO_WAIT);
            data->two_finger_was_scrolling = true;
            LOG_DBG("2F scroll LEFT");
            break;
        
        case GESTURE_SCROLL_RIGHT:
            input_report_rel(dev, INPUT_REL_HWHEEL, SCROLL_STEP, true, K_NO_WAIT);
            data->two_finger_was_scrolling = true;
            LOG_DBG("2F scroll RIGHT");
            break;

        case GESTURE_PINCH_IN:
            input_report_rel(dev, INPUT_REL_WHEEL, SCROLL_STEP * 2, true, K_NO_WAIT);
            LOG_DBG("Pinch IN (Zoom OUT)");
            break;

        case GESTURE_PINCH_OUT:
            input_report_rel(dev, INPUT_REL_WHEEL, -SCROLL_STEP * 2, true, K_NO_WAIT);
            LOG_DBG("Pinch OUT (Zoom IN)");
            break;
        default:
            LOG_DBG("2F unknown gesture: %d", gesture);
            break;
    }
}

/*
Three finger handler
*/

static void handle_three_finger(const struct device *dev, struct tps65201a_data *data, uint8_t gesture)
{
    ig
}