#include <FEH.h>
#include <Arduino.h>
#include <FEHMotor.h>
#include <FEHIO.h>
#include <FEHLCD.h>
#include <FEHSD.h>
#include <FEHRCS.h>
#include <FEHUtility.h>
#include <FEHRCS.h>
#include <FEHServo.h>

//250 counts at 12.5 at 40% motor percent
//1 encoder count per degree for turning

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

//servo arm
FEHServo arm_servo(FEHServo::Servo0); 


enum LineStates {
    MIDDLE,
    RIGHT,
    LEFT
};

void follow_optosensor(float time)
{

    LineStates state = MIDDLE;
    float timeNow = TimeNow();
    
    while (TimeNow() - timeNow < time) {

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
    int rightPercent = percent;
    int leftPercent = percent;
    //Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();

    //Set both motors to desired percent
    right_motor.SetPercent(0);
    left_motor.SetPercent(0);
    right_motor.SetPercent(rightPercent);
    left_motor.SetPercent(leftPercent);

    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((left_encoder.Counts() + right_encoder.Counts()) / 2. < counts)
    {
        /* if(left_encoder.Counts() > right_encoder.Counts()) {
            if (rightPercent<=80)
            {
                rightPercent+=5;
                leftPercent -=5;
                right_motor.SetPercent(rightPercent);
                left_motor.SetPercent(leftPercent);
            }

        } else if (right_encoder.Counts() > left_encoder.Counts()) {
            if (leftPercent<=80)
            {
                rightPercent+=5;
                leftPercent -=5;
                right_motor.SetPercent(rightPercent);
                left_motor.SetPercent(leftPercent);
            }
        } */
    }

    //Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}

void turn_right(int percent, int counts) //using encoders
{
    right_motor.SetPercent(0);
    left_motor.SetPercent(0);

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
    right_motor.SetPercent(0);
    left_motor.SetPercent(0);
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

     
    while (cds_cell.Value() > 1.2) {
        // waiting for start light to turn on
    } 
    // ─── HIT START BUTTON ─────────────────────────────────────────────────── 
    move_forward(-40, 50);  // reverse into start button


    move_forward(40, 50);   // move back forward
    // ─── NAVIGATE TO RAMP ───────────────────────────────────────────────────
    arm_servo.SetDegree(0); //arm in up position
    move_forward(40, 500); //move forward to the line following
    turn_left(20, 54); //turn right to line up with line following
    move_forward(-40, 75);
    arm_servo.SetDegree(79); //set arm to down position getting ready to pick up
  
   

    


/*

//rotate before window
    turn_left(20, 437);
    move_forward(-40, 300);
    Sleep(2);
    move_forward(30, 1000);
    move_forward(-30, 500);

 
 */ 

    
   /*  follow_optosensor();
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
     */
  
    
    
    
}
