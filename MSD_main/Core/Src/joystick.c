#include "joystick.h"
#include "stdio.h"
#include "usart.h"

extern ADC_HandleTypeDef hadc1;



int getX(void) {
    //Set ADC Channel to 1
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_13;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    //Read ADC 1
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 0xFFFFFFFF);
    return HAL_ADC_GetValue(&hadc1);
}

int getY(void) {
    //Set ADC Channel to 2
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_14;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    //Read ADC 2
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 0xFFFFFFFF);
    return HAL_ADC_GetValue(&hadc1);
}

void joystick_correct(const JoyCal *cal, float x_raw, float y_raw,
                      float *x_out, float *y_out,
                      float *mag, float *angle)
{
    float dx = x_raw - cal->x0;
    float dy = y_raw - cal->y0;

    float x = cal->a11 * dx + cal->a12 * dy;
    float y = cal->a21 * dx + cal->a22 * dy;

    float r = sqrtf(x*x + y*y);

    if (r > 1.0f) {
        x /= r;
        y /= r;
        r = 1.0f;
    }

    *x_out = x;
    *y_out = y;
    *mag = r;
    *angle = atan2f(y, x);
}

JoyCal joystick_calibrate(void)
{
    int CALIBRATE_TIME_DELAY = 2000; // Time for calibration

    // TODO: MEASURE CENTER
    printf("Keep the stick at neutral (center)\r\n");
    HAL_Delay(CALIBRATE_TIME_DELAY);
    printf("Measuring Center!\r\n");
    int centerX = getX(); // 2000
    int centerY = getY(); // 1900

    // TODO: MEASURE X_NEG
    printf("Move the stick to X_NEG (left)\r\n");
    HAL_Delay(CALIBRATE_TIME_DELAY);
    printf("Measuring X_NEG!\r\n");
    int leftX= getX();
    int leftY= getY();

    // TODO: MEASURE X_POS
    printf("Move the stick to X_POS (right)\r\n");
    HAL_Delay(CALIBRATE_TIME_DELAY);
    printf("Measuring X_POS!\r\n");
    int rightX= getX();
    int rightY= getY();

    // TODO: MEASURE Y_NEG
    printf("Move the stick to Y_NEG (down)\r\n");
    HAL_Delay(CALIBRATE_TIME_DELAY);
    printf("Measuring Y_NEG!\r\n");
    int downX= getX();
    int downY= getY();

    // TODO: MEASURE Y_POS
    printf("Move the stick to Y_POS (up)\r\n");
    HAL_Delay(CALIBRATE_TIME_DELAY);
    printf("Measuring Y_POS!\r\n");
    int upX= getX();
    int upY= getY();

    // TODO: RUN CALIBRATE FUNCTION TO GENERATE COEFFFS
    // Step 1: subtract center from each extreme point
    float relLeftX = (float)leftX - centerX;
    float relLeftY = (float)leftY - centerY;

    float relRightX = (float)rightX - centerX;
    float relRightY = (float)rightY - centerY;

    float relDownX = (float)downX - centerX;
    float relDownY = (float)downY - centerY;

    float relUpX = (float)upX - centerX;
    float relUpY = (float)upY - centerY;

    // Step 2: compute basis vectors
    // ex = (pR - pL)/2
    float eXX = (relRightX - relLeftX) * 0.5f;
    float eXY = (relRightY - relLeftY) * 0.5f;

    // ey = (pU - pD)/2
    float eYX = (relUpX - relDownX) * 0.5f;
    float eYY = (relUpY - relDownY) * 0.5f;

    // Step 4: invert B to get A
    float det = eXX * eYY - eYX * eXY;

    float a11 =  eYY / det;
    float a12 = -eYX / det;
    float a21 = -eXY / det;
    float a22 =  eXX / det;

    JoyCal joy = {
        centerX, centerY,
        a11, a12,
        a21, a22
    };

    return joy;
}


