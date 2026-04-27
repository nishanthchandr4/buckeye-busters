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



//input a negative percent if you want to move backwards
//counts: is the distance to move forwards using shaft encoder
void move_forward(int percent, int counts) 
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


    //keep motors on while there are remaining encoder counts
    while((left_encoder.Counts() + right_encoder.Counts()) / 2. < counts){} 

    //stop motors
    right_motor.Stop();
    left_motor.Stop();
}

//percent is motor percent and counts is the number of encoder counts to turn
void turn_right(int percent, int counts) 
{
    right_motor.SetPercent(0);
    left_motor.SetPercent(0);

    //Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();

    //Set both motors to desired percent
    right_motor.SetPercent(-1 * percent);
    left_motor.SetPercent(percent);
    

    //keep motors on while there are remaining encoder counts
    while((left_encoder.Counts() + right_encoder.Counts() / 2.0) < counts);

    //Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}

//percent is motor percent and counts is the number of encoder counts to turn
void turn_left(int percent, int counts) 
{
    right_motor.SetPercent(0);
    left_motor.SetPercent(0);
    //Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();
    //Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(-1 * percent);

    //keep motors on while there are remaining encoder counts
    while((left_encoder.Counts() + right_encoder.Counts() / 2.0) < counts);

    //Turn off motors

    right_motor.Stop();
    left_motor.Stop();
}

 
#define PULSE_POWER 15
#define PULSE_TIME 0.1
#define RCS_WAIT_TIME_IN_SEC 0.3
#define COUNTS_PER_INCH 20.513
#define COUNTS_PER_DEGREE 0.893
#define POWER 25
#define PLUS 0
#define MINUS 1


/*
 * Move forward using shaft encoders where percent is the motor percent and counts is the distance to travel
 */

void pulse_forward(int percent, float seconds) 
{
    // Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(percent);

    // Wait for the correct number of seconds
    Sleep(seconds);

    // Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}

 
/* 
 * Use RCS to move to the desired x_coordinate based on the orientation of the AruCo code
 */

void check_x(float x_coordinate, int orientation)
{
    // Determine the direction of the motors based on the orientation of the AruCo code 
    int power = PULSE_POWER;
    

    RCSPose* pose = RCS.RequestPosition();
    int currentCalls = RCS.RequestsRemaining();

    // Check if receiving proper RCS coordinates and whether the robot is within an acceptable range
    while( pose->x != -1 && (pose->x < x_coordinate - 0.5 || pose->x > x_coordinate + 0.5) && currentCalls - RCS.RequestsRemaining() <= 10)
        {
            if(pose->x < x_coordinate - 1)
            {
                // Pulse the motors for a short duration in the correct direction
                pulse_forward(-power, PULSE_TIME);
            }
            else if(pose->x > x_coordinate + 1)
            {
                // Pulse the motors for a short duration in the correct direction
                pulse_forward(power, PULSE_TIME);
            }
            Sleep(RCS_WAIT_TIME_IN_SEC);

            pose = RCS.RequestPosition();
    }
}

 



/* 
 * Use RCS to move to the desired heading
 */
 void check_heading(float heading, float accuracy, int calls)
{
    RCSPose* pose = RCS.RequestPosition();
    int currentCalls = RCS.RequestsRemaining();

    // Wait until a valid heading is received
    while(pose->heading == -1)
    {
        Sleep(RCS_WAIT_TIME_IN_SEC);
        pose = RCS.RequestPosition();
        
    }

    float diff = heading - pose->heading;
    if(diff > 180)  diff -= 360;
    if(diff < -180) diff += 360;

    // Use diff instead of raw heading comparison
    while(pose->heading != -1 && fabs(diff) > accuracy && currentCalls - RCS.RequestsRemaining() <= calls)
    {
        if(diff > 0)
        {
            // CCW
            right_motor.SetPercent(PULSE_POWER);
            left_motor.SetPercent(-PULSE_POWER);
        }
        else if (diff < 0)
        {
            // CW (you were missing this case!)
            right_motor.SetPercent(-PULSE_POWER);
            left_motor.SetPercent(PULSE_POWER);
        }
        else
        {
            right_motor.Stop();
            left_motor.Stop();
        }
        
        Sleep(PULSE_TIME);
        right_motor.Stop();
        left_motor.Stop();

        Sleep(RCS_WAIT_TIME_IN_SEC);
        pose = RCS.RequestPosition();

        // recompute diff every loop
        diff = heading - pose->heading;
        if(diff > 180)  diff -= 360;
        if(diff < -180) diff += 360;
    }
} 
 
void ERCMain()
{

    // Initialize RCS and wait for start light
    RCS.InitializeTouchMenu("1130D3UAI");
      
    // waiting for start light to turn on
    while (cds_cell.Value() > 1.2) {
    } 
    
    // reverse into start button
    move_forward(-40, 50);  
    //arm in up position
    arm_servo.SetDegree(0); 

   //turn and move towards composter
    turn_left(20, 80);      
    move_forward(40, 215);   

    //rotate composter bin clockwise and counter-clockwise
    float time = TimeNow();
    bin_servo.SetDegree(130);
    while (TimeNow() - time < 1.5);
    bin_servo.SetDegree(50);
    while (TimeNow() - time < 3);
    bin_servo.SetDegree(90);

    
    //move towards window
    move_forward(-40, 40);
    bin_servo.Off();
    turn_right(20, 115);
    move_forward(40,199);
    //set arm to correct position to open window
    arm_servo.SetDegree(47);
    turn_left(20, 85);
    move_forward(40, 110);
    turn_right(20, 62);

    //align with window handle
    check_heading(270, 0.8, 5);

    //move into position and move open the window
    move_forward(20, 80);
    turn_right(90, 124);
    
    //set arm back to default position
    arm_servo.SetDegree(0);

    //realign with wall under window
    move_forward(-20, 60);
    turn_left(20, 35);
    move_forward(40, 130);

    //move towards apple bucket
    move_forward(-40, 99);
    turn_left(20, 112);
    move_forward(-40, 20);

    //align with apple bucket
    check_heading(359, 0.67, 10);
    check_x(19.45, PLUS);
    check_heading(359, 0.67, 10);
    

    //move towards apple bucket to remove from tree trunk
    move_forward(-30, 80);
    arm_servo.SetDegree(93);
    move_forward(40, 130);
    
   
    Sleep(.5);
    
    //move towards opposite wall to re-align before moving up ramp
    move_forward(-20, 80);
    arm_servo.SetDegree(5);
    turn_right(20, 38);
    move_forward(-40, 320);
    turn_left(20, 36);
    move_forward(-40, 140);
    move_forward(20, 65);
    turn_right(20, 98);
    
    
    //go up the ramp and stop at the table
    move_forward(80, 760);
    move_forward(-20, 60); 
    Sleep(0.5);
    //reset arm to default position if not done already
    arm_servo.SetDegree(0);

    //turn left and re-align with wall before going to humidifier buttons
    turn_left(20, 112);
    move_forward(-40, 190);

    //move forward to humidifier buttons
    move_forward(60, 300);
    
    //check heading to ensure bumper will hit humidifier button
    check_heading(355.0, 1.0, 10.0);

    //press the humidifier button
    move_forward(40, 170);
 
    Sleep(2); 
    

   //move back and move towards lever task
    move_forward(-20, 80);
    //arm in up position
    arm_servo.SetDegree(0); 
    turn_right(20, 80);
    move_forward(20, 110);
    move_forward(20, 40);

    //arm in down position to flip lever down
    arm_servo.SetDegree(100); 
    Sleep(5);

    //move back to wall next to table before going down the ramp
    move_forward(-40, 140);
    //arm in up position
    arm_servo.SetDegree(0);
    turn_right(20, 20);
    move_forward(20, 40);
    turn_right(20, 116);
    move_forward(40, 275);
    move_forward(-40, 25);

    //turn towards the ramp and go down towards the stop button
    turn_right(40, 100);
    move_forward(60, 1200);


    
}
