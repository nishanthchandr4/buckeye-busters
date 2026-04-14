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
FEHServo bin_servo(FEHServo::Servo1);


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
    //int lever_heading = 100;
    //TestGUI();
    // ─── WAIT FOR START LIGHT ───────────────────────────────────────────────
    /*while (cds_cell.Value() > 1.2) {
        // waiting for start light to turn on
    } 
    // ─── HIT START BUTTON ─────────────────────────────────────────────────── 
    */
   //temporary hit the start button 
    move_forward(-40, 50);  // reverse into start button
    arm_servo.SetDegree(0); //arm in up position

   
    //move_forward(40, 50);   // move back forward

    /*
        Composter Task
    */
    //turn left towards composter 
    turn_left(20, 75);      // turn left to be parallel with line
    move_forward(40, 217);
    float time = TimeNow();
    bin_servo.SetDegree(130);
    while (TimeNow() - time < 1.5);
    bin_servo.SetDegree(50);
    while (TimeNow() - time < 3);
    bin_servo.SetDegree(90);

    /*
        Move to apple basket Task
    */
    move_forward(-40, 40);
    turn_right(20, 112);
    move_forward(40,299);

    /*
        Pickup apple basket
    */
    turn_left(20, 120);
    arm_servo.SetDegree(82);
    move_forward(25, 150);
    Sleep(.5);
    arm_servo.SetDegree(25);

    //turn_right(-20, 40);
    
    move_forward(-20, 80);
    turn_right(50, 65);
    move_forward(-40, 360);
    turn_left(20, 30);
    move_forward(-40, 220);


    // off wall to table
    move_forward(20, 43);
    turn_right(20, 90);
    turn_left(40, 25);
    move_forward(20, 60);
    turn_right(40, 30);
    move_forward(60, 290);
    turn_left(40, 30);
    Sleep(0.5);
    move_forward(60,290);
    Sleep(0.5);
    turn_left(40, 30);
    move_forward(20, 140);
    
    Sleep(1);
    arm_servo.SetDegree(79); //arm in down position
    Sleep(0.5);

    //move_forward(-40, 50);
    // ─── NAVIGATE TO RAMP ───────────────────────────────────────────────────
    /* arm_servo.SetDegree(0); //arm in up position
    move_forward(40, 460); //move forward to the line following
    move_forward(-40, 100); //move back a little bit to make sure optosensors can read line
    //follow_optosensor(5.0);
    //arm_servo.SetDegree(79); //set arm to down position getting ready to pick up
    turn_left(20, 51);
    move_forward(-40, 100);
    arm_servo.SetDegree(79); //arm in down position
    Sleep(0.5);
    move_forward(20, 148);
    Sleep(0.5);
    //turn_left(20, 20);
    /*for (int i = 0; i < 5; i++) {
        arm_servo.SetDegree(79);
        Sleep(0.5);
    }*/
    /*arm_servo.SetDegree(25); //arm in up position
    //turn_right(-20, 40);
    move_forward(-20, 80);
    turn_right(20, 30);
    move_forward(-40, 320);
    turn_left(20, 15);
    move_forward(-40, 260);


    // off wall to table
    move_forward(20, 43);
    turn_right(20, 90);
    turn_left(40, 25);
    move_forward(20, 60);
    turn_right(40, 30);
    move_forward(60, 520);
    Sleep(0.5);
    turn_left(40, 15);
    move_forward(20, 100);
    
    Sleep(1);
    arm_servo.SetDegree(79); //arm in down position
    Sleep(0.5);
    move_forward(-40, 100);
    turn_left(20, 90);
    move_forward(40, 380);
    turn_right(20,45);
    arm_servo.SetDegree(25); //arm in up position
    //move_forward(40, 200);
    arm_servo.SetDegree(79); //arm in down position
    move_forward(-40, 100);
    move_forward(40, 100);
    arm_servo.SetDegree(25); //arm in up position
    Sleep(0.5);

    /* if (RCS.GetLever() == 0) { //left lever
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


*/


/*

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
