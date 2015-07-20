/* DragonBot 2015
s *
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
	static const int MAX_EXCESS_SMOKE_TIME = 2;
	static constexpr float SMOKE_CANNON_SPEED = 0.4f;
	Victor *smoke_cannon;
	DigitalOutput *smoke_machine;
	Timer *smoke_make_timer;
	Timer *smoke_fire_timer;

	static constexpr float WING_FOLD_SPEED = 0.8;
	static constexpr float WING_FLAP_SPEED = 0.4;

	static const int SOUND_PIN = 2;
	std::vector<int> sound_seq;
	DigitalOutput *sound_out;

	RobotDrive *drive;

	GamepadF310 *pilot;
	GamepadF310 *copilot;

	Victor *jaw;
	Victor *head;
	Victor *wing_flap;
	Victor *wing_fold;

	float eye_angle;
	Servo *eye;

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
		smoke_machine = new DigitalOutput(SMOKE_MACHINE_RELAY);
		smoke_make_timer = new Timer();
		smoke_fire_timer = new Timer();

		jaw = new Victor(JAW_MOTOR_PWM);
		head = new Victor(HEAD_MOTOR_PWM);
		wing_flap = new Victor(WING_FLAP_PWM);
		wing_fold = new Victor(WING_FOLD_PWM);

		eye = new Servo(EYE_PWM);
		eye_angle = 0;
	}

	void setSound(DigitalOutput *out, bool state) {
		out->Set(!state);
	}


	void DisabledInit() {
		smoke_make_timer->Stop();
		smoke_fire_timer->Stop();
		smoke_make_timer->Reset();
		smoke_fire_timer->Reset();
	}

	void DisabledPeriodic()
	{
		setSound(sound_out, false);
	}

	void AutonomousInit() {}
	void AutonomousPeriodic() {}

	void TeleopInit()
	{

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

        if(copilot->ButtonState(GamepadF310::buttonStart)){
        	wing_fold->Set(WING_FOLD_SPEED);
        } else if(copilot->ButtonState(GamepadF310::buttonBack)) {
        	wing_fold->Set( -WING_FOLD_SPEED );
        } else {
        	wing_fold->Set(0.0);
        }

        if(copilot->ButtonState(GamepadF310::buttonB)){
        	wing_flap->Set(WING_FLAP_SPEED);
        } else {
        	wing_flap->Set(0.0);
        }

        // left trigger/button control head and jaw
        // right trigger/button control jaw only

        bool left_down = copilot->LeftTrigger() > 0.5;
        bool left_up = copilot->ButtonState(GamepadF310::buttonLeftBumper);
        bool right_down = copilot->RightX() >= 0.5f;
        bool right_up = copilot->ButtonState(GamepadF310::buttonRightBumper);

        if ((int)left_down + (int)left_up + (int)right_down + (int)right_up != 1) {
        	// either no buttons are pressed or multiple,
        	// conflicting buttons are pressed
        	jaw->Set(0);
        	head->Set(0);
        } else if (left_down) {
        	jaw->Set(0.2);
        	head->Set(-0.3);
        } else if (left_up) {
        	jaw->Set(-0.4);
        	head->Set(0.5);
        } else if (right_down){
        	jaw->Set(0.4);
        } else if (right_up){
        	jaw->Set(-0.4);
        }

        if (copilot->ButtonState(GamepadF310::buttonA)) {
        	eye_angle = ((1 - copilot->LeftX()) * 60) + 50;
        }
        eye->SetAngle(eye_angle);

        if (copilot->ButtonState(GamepadF310::buttonX)) {
        	// make smoke
        	if (smoke_make_timer->Get() - smoke_fire_timer->Get() < MAX_EXCESS_SMOKE_TIME) {
        		smoke_machine->Set(true);
        		SmartDashboard::PutString("smoke machine", "active");
        	} else {
        		smoke_machine->Set(false);
        		SmartDashboard::PutString("smoke machine", "maximum");
        	}
        	smoke_make_timer->Start();
        }
        else {
        	smoke_machine->Set(false);
        	SmartDashboard::PutString("smoke machine", "inactive");
        	smoke_make_timer->Stop();
        }

        if (copilot->ButtonState(GamepadF310::buttonY)) {
        	// shoot smoke
        	smoke_cannon->Set(SMOKE_CANNON_SPEED);
			if (smoke_make_timer->Get() > smoke_fire_timer->Get()){
				//measure how long we've fired smoke, so we know if it's ok to make more
				smoke_fire_timer->Start();
	        	SmartDashboard::PutString("smoke cannon", "active");
			} else {
				smoke_fire_timer->Stop();
	        	SmartDashboard::PutString("smoke cannon", "stopped");
			}

        } else {
            smoke_cannon->Set(0);
        	SmartDashboard::PutString("smoke cannon", "inactive");
        }

		//if both timers are the same, we can set them both to zero to ensure we don't overflow them or something
		if (smoke_make_timer->Get() == smoke_fire_timer->Get()){
			smoke_make_timer->Reset();
			smoke_fire_timer->Reset();
		}

	}

	void TestInit() {}

	void TestPeriodic() {}
};

START_ROBOT_CLASS(Robot);
