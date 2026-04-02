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

//24.5 inches equal 1000 counts at 40%

// Declare things like Motors, Servos, etc. here
// For example:
// FEHMotor leftMotor(FEHMotor::Motor0, 6.0);
// FEHServo servo(FEHServo::Servo0);

// Declarations for motor encoders
DigitalEncoder right_encoder (FEHIO:: Pin13);
DigitalEncoder left_encoder (FEHIO:: Pin14);

//declarations for motors
FEHMotor right_motor(FEHMotor:: Motor3, 9.0);
FEHMotor left_motor(FEHMotor:: Motor0, 9.0);

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
    
    while (timeNow - TimeNow() >= time) {

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
#define PULSE_POWER 20
#define PULSE_TIME 0.1
#define RCS_WAIT_TIME_IN_SEC 0.3

void check_heading(float heading)
{
    RCSPose* pose = RCS.RequestPosition();

    // Wait until a valid heading is received
    while(pose->heading == -1)
    {
        Sleep(RCS_WAIT_TIME_IN_SEC);
        pose = RCS.RequestPosition();
    }

    // Keep correcting until within 2 degrees of target
    while(pose->heading != -1 && (pose->heading < heading - 2 || pose->heading > heading + 2))
    {
        // Special case: handle wrap-around near 0/360 degrees
        // e.g. target is 0, robot reads 355 — that's only 5 degrees off, not 355
        float diff = heading - pose->heading;
        if(diff > 180)  diff -= 360;
        if(diff < -180) diff += 360;

        if(diff > 0)
        {
            // Need to turn counterclockwise
            right_motor.SetPercent(PULSE_POWER);
            left_motor.SetPercent(-PULSE_POWER);
        }
        

        Sleep(PULSE_TIME);
        right_motor.Stop();
        left_motor.Stop();

        Sleep(RCS_WAIT_TIME_IN_SEC);
        pose = RCS.RequestPosition();
    }
}


void ERCMain()
{
    //RCS.GetLever();
    int lever_heading = 100;

    // ─── WAIT FOR START LIGHT ───────────────────────────────────────────────
    //while (cds_cell.Value() > 1.2) {
        // waiting for start light to turn on
    //} 
    // ─── HIT START BUTTON ───────────────────────────────────────────────────
    move_forward(-40, 50);  // reverse into start button
    move_forward(40, 55);   // move back forward

    // ─── NAVIGATE TO RAMP ───────────────────────────────────────────────────
    /*arm_servo.SetDegree(0); //arm in up position
    move_forward(40, 490); //move forward to the line following
    follow_optosensor(5.0);
    arm_servo.SetDegree(79); //set arm to down position getting ready to pick up
    move_forward(40, 100);
    turn_left(20, 20);
    arm_servo.SetDegree(25); //arm in up position
    turn_right(-20, 40);
    */

    // off wall to table
    turn_right(20, 90);
    move_forward(40, 660);
    move_forward(-40, 100);
    turn_left(20, 90);
    if (RCS.GetLever() == 0) { //left lever
        move_forward(40, 380);
        turn_right(20, 45);
        check_heading(lever_heading);
    } else if (RCS.GetLever() == 1) { //middle lever
        move_forward(40, 280);
        turn_right(20, 45);
        check_heading(lever_heading);

    } else if (RCS.GetLever() == 2) { //right lever
        move_forward(40, 160);
        turn_right(20, 45);
        check_heading(lever_heading);
    }





//rotate before window
    /* turn_left(20, 437);
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
