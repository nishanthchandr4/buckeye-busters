#include <FEH.h>
#include <Arduino.h>
#include <FEHMotor.h>
#include <FEHIO.h>
#include <FEHLCD.h>
#include <FEHSD.h>
#include <FEHRCS.h>
#include <FEHUtility.h>

//24.5 inches equal 1000 counts at 40%

// Declare things like Motors, Servos, etc. here
// For example:
// FEHMotor leftMotor(FEHMotor::Motor0, 6.0);
// FEHServo servo(FEHServo::Servo0);

// Declarations for motor encoders
DigitalEncoder right_encoder (FEHIO:: Pin13);
DigitalEncoder left_encoder (FEHIO:: Pin14);

//declarations for motors
FEHMotor right_motor(FEHMotor:: Motor0, 9.0);
FEHMotor left_motor(FEHMotor:: Motor1, 9.0);

//declarations for CdS Cell
AnalogInputPin cds_cell(FEHIO::Pin3);

//declarations for optosensors
AnalogInputPin left_opto(FEHIO::Pin2);
AnalogInputPin middle_opto(FEHIO::Pin1);
AnalogInputPin right_opto(FEHIO::Pin0);


enum LineStates {
    MIDDLE,
    RIGHT,
    LEFT
};

void follow_optosensor()
{
    LineStates state = MIDDLE;
    
    while (true) {

        bool rightOnTape  = right_opto.Value()  > 4.0;
        bool leftOnTape   = left_opto.Value()   > 4.0;
        bool middleOnTape = middle_opto.Value() > 4.0;

        if (rightOnTape && leftOnTape && middleOnTape) {
            right_motor.Stop();
            left_motor.Stop();
            return;
        }

        switch (state) {

            case MIDDLE:
                right_motor.SetPercent(20);
                left_motor.SetPercent(20);

                if (right_opto.Value() > 3.6) {
                    state = RIGHT;
                } else if (left_opto.Value() > 3.6) {
                    state = LEFT;
                }
                break;

            case RIGHT:
                right_motor.SetPercent(5);
                left_motor.SetPercent(30);

                if (right_opto.Value() < 4.0) {
                    state = MIDDLE;
                }
                break;

            case LEFT:
                right_motor.SetPercent(30);
                left_motor.SetPercent(5);

                if (left_opto.Value() < 4.0) {
                    state = MIDDLE;
                }
                break;

            default:
                right_motor.Stop();
                left_motor.Stop();
                break;
        }
        Sleep(0.02);
    }

    right_motor.Stop();
    left_motor.Stop();
}

//input a negative percent if you want to move backwards
void move_forward(int percent, int counts) //using encoders
{
    //Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();

    //Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(percent);

    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((left_encoder.Counts() + right_encoder.Counts()) / 2. < counts);

    //Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}

void turn_right(int percent, int counts) //using encoders
{
    //Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();

    //Set both motors to desired percent
    right_motor.SetPercent(-1 * percent);
    left_motor.SetPercent(percent);
    //hint: set right motor backwards, left motor forwards

    //<ADD CODE HERE>
    //While the average of the left and right encoder is less than counts,
    while((left_encoder.Counts() + right_encoder.Counts() / 2.0) < counts);

    //Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}

void turn_left(int percent, int counts) //using encoders
{
    //Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();
    //Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(-1 * percent);

    while((left_encoder.Counts() + right_encoder.Counts() / 2.0) < counts);

    //Turn off motors

    right_motor.Stop();
    left_motor.Stop();
}


void test1()
{
    //move
        move_forward(40, 1285); 
}
void test2()
{
    move_forward(40, 1200);
    //turn_right(20, 850);
    move_forward(-40, 1100);
}

void ERCMain()
{

    // ─── WAIT FOR START LIGHT ───────────────────────────────────────────────
    while (cds_cell.Value() > 1.2) {
        // waiting for start light to turn on
    } 
    // ─── HIT START BUTTON ───────────────────────────────────────────────────
    move_forward(-40, 50);  // reverse into start button
    move_forward(40, 55);   // move back forward

    // ─── NAVIGATE TO RAMP ───────────────────────────────────────────────────
    turn_right(20, 219);
    move_forward(40, 1700);
    turn_left(20, 437);
    move_forward(-40, 300);
    Sleep(2);
    move_forward(30, 440);
    follow_optosensor();
    LCD.Write("finished following optosensor");
    move_forward(40, 139);

    Sleep(2); 
    

    float cdsValue = cds_cell.Value();
    LCD.Write("cds value: ");
    LCD.Write(cdsValue);

    if(cdsValue < 0.48) { //red light
        LCD.Write("red light: ");
        LCD.Write(cdsValue);
        turn_right(20,219);
        move_forward(40, 50);
        turn_left(20, 205);
        move_forward(40, 244);
    } else if(cds_cell.Value() > 0.48 && cds_cell.Value() < 1.0) { //blue light
        LCD.Write("blue light: ");
        LCD.Write(cdsValue);
        turn_left(20, 219);
        move_forward(40, 20);
        turn_right(20, 219);
        move_forward(40, 244);
    } 
    move_forward(-40, 1061);
    move_forward(40, 60);
    turn_left(40, 490);
    //turn_right(20, 219);
    //move forwARD
    //MOVERIGHT
    move_forward(80, 2100);
    turn_left(100, 200);
    
  
    
    
    
}
