#include <DualVNH5019MotorShield.h>
#include <EnableInterrupt.h>
#include "RobotMotor.h"
#include "RobotSensor.h"
#include <math.h>
#include <SharpIR.h>

RobotMotor motor;

/*==
2YK Sensors range 20-150cm (Big sensor)

21YK Sensor range 10-80cm (Small Sensor)
==*/

//GP2Y0A21YK0F = short 10cm to 80cm
//GP2Y0A2YK0F = long 20cm to 150cm
SharpIR ir_FL(GP2Y0A21YK0F, A0);
SharpIR ir_FR(GP2Y0A21YK0F, A1);
SharpIR ir_FC(GP2Y0A21YK0F, A2); //Front center
SharpIR ir_SF(GP2Y0A21YK0F, A3); //Right Forward
SharpIR ir_SB(GP2Y0A21YK0F, A4); //Right Back
SharpIR ir_LM(GP2Y0A02YK0F, A5);

//------------------Motor Pins ---------------------//
const int M1A = 3;
const int M1B = 5;
const int M2A = 11;
const int M2B = 13;

//Encoder rising edge tick++
void m1Change()
{
	motor.M1Change();
}

void m2Change()
{
	motor.M2Change();
}

void RPMBenchtest()
{
	int setSpeed = 200;
	for (int i = 0; i < 6; ++i)
	{
		motor.GetMotor().setSpeeds(setSpeed, setSpeed);
		delay(1005);

		motor.CalcRPM();
		motor.GetMotor().setBrakes(400, 400);
		delay(1005);
	}

	motor.GetMotor().setBrakes(400, 400);
}

void CalibrationRPM()
{
	motor.CalibrationForward(80);
}

//Makes both sensors aligned with each other i.e. have both sensors measure same distance from the front
void Calibrate_FrontAngle()
{
	double leftSensor;
	double rightSensor;
	for (int i = 0; i < 10; ++i)
	{
		//Align both sensors forward first, regardless whether its far or near
		leftSensor = ir_FL.getDistance();
		rightSensor = ir_FR.getDistance();

		double sensorDiff = abs(leftSensor - rightSensor);

		if (sensorDiff <= 0.0)
			break;

		//Atan returns radian. Convert rad to deg by multiplying 180/PI
		double angle = atan(sensorDiff / 14) * 180 / PI;


		//Left is further
		if (leftSensor > rightSensor)
		{
			//Turn right
			//Serial.print("   Turn Right");
			motor.Turn(angle);

		}
		else //Right is further
		{
			//Turn left
			//Serial.print("   Turn Left");
			motor.Turn(-angle);
		}
	}

}

//Calibrates right side to align straight
void Calibrate_SideAngle()
{
	double leftSensor;
	double rightSensor;
	for (int i = 0; i < 10; ++i)
	{
		//Align both sensors forward first, regardless whether its far or near
		leftSensor = ir_SF.getDistance();
		rightSensor = ir_SB.getDistance();

		double sensorDiff = abs(leftSensor - rightSensor);

		if (sensorDiff <= 0.0)
			break;

		//Atan returns radian. Convert rad to deg by multiplying 180/PI
		double angle = atan(sensorDiff / 14) * 180 / PI;


		//Left is further
		if (leftSensor > rightSensor)
		{
			//Turn right
			//Serial.print("   Turn Right");
			motor.Turn(angle);

		}
		else //Right is further
		{
			//Turn left
			//Serial.print("   Turn Left");
			motor.Turn(-angle);
		}
	}

	delay(200);
}

//Move forward until one sensor reads setPoint distance
void Calibrate_Forward(int setPoint)
{

	for (int i = 0; i < 5; ++i)
	{
		double frontSensor = ir_FC.getDistance();

		//Distance between robot and setpoint
		//If negative, it means that the robot is too near from the setpoint
		//If positive, it means that the robot is too far from the setpoint
		double distance = frontSensor - setPoint;

		if (distance == 0)
			break;

		if (distance > 0) //Robot away from setpoint
		{
			//Absolute distance must be passed in as a negative value will mess up the Forward function
			//to indicate reverse instead, we pass true in the second argument
			motor.Forward(abs(distance), false);
		}
		else if (distance < 0) //Robot too 
		{
			motor.Forward(abs(distance), true);
		}
	}
}

void Calibrate_Full(int setPoint)
{
	//Calibrate_Angle(setPoint);

	//while (!Calibrate_Forward(setPoint))
	//{
	//	Calibrate_Angle(setPoint);
	//}

}

void CalibrationTest()
{
	Calibrate_FrontAngle();
	delay(100);

	Calibrate_Forward(15);
	delay(100);

	//Turn left
	motor.TurnLeft90();
	delay(100);


	//Calibrate
	Calibrate_FrontAngle();
	delay(100);

	Calibrate_Forward(15);
	delay(100);


	//Turn left
	motor.TurnRight90();
	delay(100);


	//Calibrate
	Calibrate_FrontAngle();
	delay(100);

	Calibrate_Forward(15);
	delay(100);

	//Turn left
	motor.TurnLeft90();
	delay(100);


	//Calibrate
	Calibrate_FrontAngle();
	delay(100);

	Calibrate_Forward(15);
	delay(100);

	motor.TurnLeft90();
	delay(100);

	Calibrate_SideAngle();
}

void Checklist_Obstacle90(int distance)
{
	int travelled = ((distance) / 10);
	travelled -= 5;

	CalibrationTest();

	while (ir_FC.getDistance() > 15)
	{
		motor.Forward10();
		--travelled;
	}



	//Obstacle Avoidance Start
	Calibrate_SideAngle();
	Calibrate_Forward(15);

	motor.Forward10();

	motor.TurnLeft90();

	motor.Forward10();

	motor.Forward10();

	motor.TurnRight90();
	//Calibrate_SideAngle();


	for (int i = 0; i < 4; ++i)
	{
		motor.Forward10();
	}

	motor.TurnRight90();


	motor.Forward10();

	motor.Forward10();

	Calibrate_FrontAngle();

	Calibrate_Forward(15);

	motor.TurnLeft90();

	Calibrate_SideAngle();

	//Obstacle Avoidance End
	while (travelled > 0)
	{
		motor.Forward10();
		--travelled;
	}
}

void Checklist_Obstacle45(int distance)
{
	CalibrationTest();
	int travelled = distance / 10;

	travelled -= 6;


	while (ir_FC.getDistance() > 10)
	{
		motor.Forward10();
		--travelled;
	}

	Calibrate_Forward(16);

	//Diagonal turning left to avoid obstacle
	//Start
	motor.TurnLeft45();
	motor.Forward10();
	motor.Forward10();
	motor.Forward10();
	motor.TurnRight45();

	motor.Forward10();
	motor.Forward10();

	motor.TurnRight45();

	motor.Forward10();
	motor.Forward10();
	motor.Forward10();

	motor.TurnRight45();
	//end

	//Behind the block now, making sure we're still along our intended line by calibrating and using the obstacle for ref
	//Start
	Calibrate_Forward(15);

	motor.TurnRight90();

	Calibrate_Forward(15);

	motor.TurnLeft90();
	motor.TurnLeft90();

	Calibrate_SideAngle();
	//End

	while (travelled > 0)
	{
		motor.Forward10();
		--travelled;
	}



}

void SendSensorValues()
{
	delay(500); //Delay in reading senosr
	int fl = ir_FL.getDistance();
	int fc = ir_FC.getDistance();
	int fr = ir_FR.getDistance();

	int lm = ir_LM.getDistance();
	int rf = ir_SF.getDistance();
	int rb = ir_SB.getDistance();

	//Short range
	
	if (fc <= 18)
	{
		fc = 1;
	}
	else if (fc >= 18 && fc < 25)
	{
		fc = 2;
	}
	else if (fc >= 25 && fc < 32)
	{
		fc = 3;
	}
	else
	{
		fc = -1;
	} 


	//Long range
	if (lm <= 19)
	{
		lm = 1;
	}
	else if (lm > 19 && lm < 30)
	{
		lm = 2;
	}
	else if (lm >= 30 && lm < 38)
	{
		lm = 3;
	}
	else if (lm >= 38 && lm < 50)
	{
		lm = 4;
	}
	else if (lm >= 50 && lm <= 60)
	{
		lm = 5;
	}
	else
	{
		lm = -1;
	}





	Serial.println("P" + String(fl) + "," + String(fc) + "," + String(fr) + "," + String(rf) + "," + String(rb)  + "," + String(lm) );
	//SENSOR|17|15|17|30|20|20

}

void setup()
{
	Serial.begin(9600);

	enableInterrupt(M1A, m1Change, CHANGE);
	enableInterrupt(M1B, m1Change, CHANGE);

	enableInterrupt(M2A, m2Change, CHANGE);
	enableInterrupt(M2B, m2Change, CHANGE);

	motor.begin();
	
	//Checklist_Obstacle90(100);
	//Checklist_Obstacle45(100);
	//motor.ForwardChecklist(150);
	//motor.Turn(1080);

	for(int i = 0; i < 3; ++ i) motor.Forward10();
}


String commands;
int currIndex = 0; //Current command index
int commandLength = 0;

void loop()
{
	if (Serial.available()) //Checks how many bytes available. sizeof(char) = 1 byte
	{
		while (Serial.available() > 0)
		{
			char temp = Serial.read();
			commands += temp; //Adds the character into the string

		}
	}

	for (commandLength = 0; commands[commandLength] != '\0'; ++commandLength);


	while (currIndex < commandLength)
	{

		char command = commands[currIndex++]; //Get command from String by treating it as an array

		switch (command)
		{
			//WASD is for movement
		case 'w':
			motor.Forward10();
			Serial.println("PY");
			break;
		case 'W':
			motor.Forward30();
			Serial.println("PY");
			break;
		case 'q':
			motor.Forward50();
			Serial.println("PY");
			break;
		case 's': //Reverse
			motor.Turn180();
			Serial.println("PY");
			break;
		case 'a': //Turn left 90
			motor.TurnLeft90();
			Serial.println("PY");
			break;
		case 'd': //Turn right 90
			motor.TurnRight90();
			Serial.println("PY");
			break;

			//TFGH is for calibration
		case 't':
			Calibrate_Forward(13);
			Serial.println("PY");
			break;
		case 'g':
			Calibrate_FrontAngle();
			Serial.println("PY");
			break;
		case 'h':
			Calibrate_SideAngle();
			Serial.println("PY");
			break;

			//IJL are for sensor readings
		case 'k':
			SendSensorValues();
			break;
		default:
			Serial.println("Error");
			break;
		}
	}
}


