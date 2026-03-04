#include <FEH.h>
#include <Arduino.h>
#include <FEHMotor.h>

// Declare things like Motors, Servos, etc. here
// For example:
// FEHMotor leftMotor(FEHMotor::Motor0, 6.0);
// FEHServo servo(FEHServo::Servo0);

// Declarations for analog optosensors
DigitalEncoder right_encoder (FEHIO:: Pin13);
DigitalEncoder left_encoder (FEHIO:: Pin14);

FEHMotor right_motor(FEHMotor:: Motor0, 9.0);
FEHMotor left_motor(FEHMotor:: Motor1, 9.0);

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

void test1()
{
    //move
        move_forward(40, 100); 
}
void test2()
{
    move_forward(60, 100);
    turn_right(20, 50);
    move_forward(30, 100);
}

void ERCMain()
{
        
    test1();


    
    
    //test2();

}