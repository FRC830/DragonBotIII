/*
 * Redoing code for DragonBot
 * For Summer 2015 so it uses eclipse
 */

#include "WPILib.h"
#include "../util/830utilities.h"

class Robot: public IterativeRobot
{
private:

	//PWM's
	static const int FRONT_LEFT_PWM = 1;
	static const int FRONT_RIGHT_PWM = 2;
	static const int BACK_LEFT_PWM = 3;
	static const int BACK_RIGHT_PWM = 4;
	static const int SMOKE_CANNON_PWM = 5;
	static const int JAW_MOTOR_PWM = 6;
	static const int HEAD_MOTOR_PWM = 7;
	static const int LEFT_EYE_X_PWM = 8;
	static const int RIGHT_EYE_X_PWM = 9;
	static const int SMOKE_MACHINE_RELAY = 1;

//	static const int TOPHEAD_LIMIT_PIN = 4;
//	static const int BOTTOMJAW_LIMIT_PIN = 2;
//	static const int CRASH_LIMIT_PIN = 3;

	static const int SOUND_PIN = 2;
	std::vector<int> sound_seq;
	DigitalOutput *sound_out;

	RobotDrive *drive;

	GamepadF310 *pilot;

public:
	void RobotInit()
	{
		drive = new RobotDrive(
				new Victor(FRONT_LEFT_PWM),
				new Victor(BACK_LEFT_PWM),
				new Victor(FRONT_RIGHT_PWM),
				new Victor(BACK_RIGHT_PWM)
		);

		pilot = new GamepadF310(0);
		sound_out = new DigitalOutput(SOUND_PIN);
		sound_out->Set(0);
	}

	void setSound(DigitalOutput *out, bool state) {
		out->Set(!state);
	}

	void DisabledInit() {}
	void DisabledPeriodic()
	{
		setSound(sound_out, false);
	}
	void AutonomousInit() {}
	void AutonomousPeriodic() {}

	void TeleopInit()
	{
		SmartDashboard::PutString("string", "this is a string");

	}

	void TeleopPeriodic()
	{
		static Timer sound_timer;
		static GamepadF310::ButtonEvent evt;
        float x = pilot->LeftX();
        float y = pilot->LeftY();
        float rot = pilot->RightX();

        drive->MecanumDrive_Cartesian(x,y,rot);

        while (pilot->GetButtonEvent(&evt)) {
        	if (evt.button == GamepadF310::buttonA && evt.pressed)
        		sound_timer.Start();
        }

		setSound(sound_out, sound_timer.Get() > 0);
        if (sound_timer.Get() > 0.1) {
        	sound_timer.Stop();
        	sound_timer.Reset();
        }
	}

	void TestInit() {}

	void TestPeriodic() {}
};

START_ROBOT_CLASS(Robot);
