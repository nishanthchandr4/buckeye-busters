#include <FEH.h>
#include <Arduino.h>
#include <FEHMotor.h>
#include <FEHIO.h>
#include <FEHLCD.h>
#include <FEHSD.h>
#include <FEHRCS.h>
#include <FEHUtility.h>

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
    //wait till cds cell reads input from the floor starting 

    //move backwards immediatly to click the start button

    //drive up the ramp to the second floor and initiate line folowwing

    //flow the lines and once all three optosensors are underblack tape

    //move set amount using shaft encoding to read the light on the floor

    //determine the color of the light and hit the correct button

    //drive back to the bottom floor and hit the final button. 

    
    //test 1 going to the other wall comxment this out when doing test 2
    //test1();
 
    //test 2 going up and down the ramp comment this out when doing test 1
    //test2();

    //move this to a different file adin everything bellow this comment
    //exploration 3 code

    // Declare variables.
    // For touchscreen functionality.
    int touch_x, touch_y;
    // Loop n through points to record necessary positions.
    int n;
    char points[4] = {'A', 'B', 'C', 'D'};

    // Disable RCS position rate limiting to allow for more
    // frequent position updates (testing only, won't work during competition)
    RCS.DisableRateLimit();

    // Call this function to initialize the RCS to a course.
    // For Exploration 3, please use the key provided below.
    //     This matches the AruCo code that is provided for this exploration.
    // If your team wishes to use RCS's positioning system going forward,
    //     please use the 8 digit code on your team's page on the store website.
    RCS.InitializeTouchMenu("D3TESTING");

    // Open SD file for writing
    FEHFile *fptr = SD.FOpen("test.txt", "w");

    // Wait for touchscreen to be pressed and released
    LCD.WriteLine("Press Screen to Start");
    while (!LCD.Touch(&touch_x, &touch_y));
    while (LCD.Touch(&touch_x, &touch_y));

    // Clear screen
    LCD.Clear();

    // Step through each path point (A,B,C,D) to record position and heading
    for (n = 0; n <= 3; n++)
    {
        LCD.Clear();
        LCD.WriteRC("Touch to set point ", 0, 0);
        LCD.WriteRC(points[n], 0, 20);

        RCSPose *pose;

        // Loop until touchscreen is pressed, continuously requesting 
        // RCS position data and writing it to the LCD screen
        while (!LCD.Touch(&touch_x, &touch_y))
        {
            // Request position data from RCS
            pose = RCS.RequestPosition();

            // Clear previous position data from LCD
            LCD.SetFontColor(BLACK);
            LCD.FillRectangle(130, 20, 100, 60);

            LCD.SetFontColor(WHITE);
            LCD.WriteRC("X Position:", 2, 0);
            LCD.WriteRC("Y Position:", 3, 0);
            LCD.WriteRC("Heading:", 4, 0);

            // Write the RCS data to the LCD screen
            LCD.WriteRC(pose->x, 2, 12);
            LCD.WriteRC(pose->y, 3, 12);
            LCD.WriteRC(pose->heading, 4, 12);
        }

        // Print RCS data for this path point to file
        SD.FPrintf(fptr, "%f %f\n", pose->x, pose->y);

        // Wait for touchscreen to be released
        while (!LCD.Touch(&touch_x, &touch_y));
    }

    // Close SD file
    SD.FClose(fptr);
}
