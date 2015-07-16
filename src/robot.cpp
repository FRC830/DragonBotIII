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
	static const int EYE_PWM = 0;
	static const int FRONT_LEFT_PWM = 1;
	static const int FRONT_RIGHT_PWM = 2;
	static const int BACK_LEFT_PWM = 3;
	static const int BACK_RIGHT_PWM = 4;
	static const int SMOKE_CANNON_PWM = 5;
	static const int JAW_MOTOR_PWM = 6;
	static const int HEAD_MOTOR_PWM = 7;
	static const int WING_FLAP_PWM = 8;
	static const int WING_FOLD_PWM = 9;

	static const int SMOKE_MACHINE_RELAY = 1;

	//Speeds
	static constexpr float WING_FOLD_SPEED = 0.9;
	static constexpr float WING_FLAP_SPEED = 0.5;

	static const int SOUND_PIN = 2;
	std::vector<int> sound_seq;
	DigitalOutput *sound_out;

	RobotDrive *drive;

	GamepadF310 *pilot;
	GamepadF310 *copilot;

	Victor *smoke_cannon;
	Victor *jaw;
	Victor *head;
	Victor *wing_flap;
	Victor *wing_fold;

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
		copilot = new GamepadF310(1);

		sound_out = new DigitalOutput(SOUND_PIN);
		sound_out->Set(0);

		smoke_cannon = new Victor(SMOKE_CANNON_PWM);
		jaw = new Victor(JAW_MOTOR_PWM);
		head = new Victor(HEAD_MOTOR_PWM);
		wing_flap = new Victor(WING_FLAP_PWM);
		wing_fold = new Victor(WING_FOLD_PWM);
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

        if(copilot->ButtonState(GamepadF310::buttonStart)){
        	wing_fold->Set(WING_FOLD_SPEED);
        } else if(copilot->ButtonState(GamepadF310::buttonBack)) {
        	wing_fold->Set( -WING_FOLD_SPEED );
        }


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
