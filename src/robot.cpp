/* DragonBot 2015
s *
 */

#include "WPILib.h"
#include "Lib830.h"

using namespace Lib830;

template <typename T>
T clamp(T x, T min, T max) {
	if (x < min) return min;
	else if (x > max) return max;
	else return x;
}

struct sound_desc {const char *name; int pin;};
static const sound_desc SOUNDS[] = {
		{"engine", 1},
		{"roar", 2},
		{"growl", 3},
		{"sheep", 4},
		{"fart", 5},
		{"laser", 6},
		{"elevator", 7},
		{"cat", 8}
};


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

	static const int SMOKE_MACHINE_DIO = 9;
	static const int MAX_EXCESS_SMOKE_TIME = 5;
	static constexpr float SMOKE_CANNON_SPEED = 0.4f;
	Victor *smoke_cannon;
	DigitalOutput *smoke_machine;
	Timer *smoke_make_timer;
	Timer *smoke_fire_timer;
	Timer *rumble_timer;

	static constexpr float WING_FOLD_SPEED = 0.8;
	static constexpr float WING_FLAP_SPEED = 0.4;
	static constexpr float WING_FLAP_ACCEL = 0.01;
	float wing_speed;

	std::map<std::string, DigitalOutput*> sound_outputs;
	std::map<int /* button ID */, SendableChooser*> sound_choosers;

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
		drive->SetInvertedMotor(RobotDrive::kFrontRightMotor, true);
		drive->SetInvertedMotor(RobotDrive::kRearRightMotor, true);

		pilot = new GamepadF310(0);
		copilot = new GamepadF310(1);

		int sound_pins = sizeof(SOUNDS) / sizeof(sound_desc);
		for (int i = 0; i < sound_pins; i++) {
			sound_outputs[SOUNDS[i].name] = new DigitalOutput(SOUNDS[i].pin);
		}

		sound_choosers[GamepadF310::BUTTON_A] = new SendableChooser();
		sound_choosers[GamepadF310::BUTTON_B] = new SendableChooser();

		for (auto it = sound_choosers.begin(); it != sound_choosers.end(); ++it) {
			SendableChooser *c = it->second;
			c->AddDefault(" none", NULL);
			for (auto out = sound_outputs.begin(); out != sound_outputs.end(); ++out) {
				c->AddObject(out->first.c_str(), out->second);
			}
		}

		SmartDashboard::PutData("sound A", sound_choosers[GamepadF310::BUTTON_A]);
		SmartDashboard::PutData("sound B", sound_choosers[GamepadF310::BUTTON_B]);


		smoke_cannon = new Victor(SMOKE_CANNON_PWM);
		smoke_machine = new DigitalOutput(SMOKE_MACHINE_DIO);
		smoke_make_timer = new Timer();
		smoke_fire_timer = new Timer();

		rumble_timer = new Timer();

		jaw = new Victor(JAW_MOTOR_PWM);
		head = new Victor(HEAD_MOTOR_PWM);
		wing_flap = new Victor(WING_FLAP_PWM);
		wing_fold = new Victor(WING_FOLD_PWM);
		wing_speed = 0;

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
		pilot->RumbleRight(0);
		pilot->RumbleLeft(0);		
	}

	void DisabledPeriodic()
	{
		for (auto it = sound_outputs.begin(); it != sound_outputs.end(); ++it) {
			setSound(it->second, false);
		}
		CommonPeriodic();
	}

	void AutonomousInit() {}
	void AutonomousPeriodic() {
		setSound(sound_outputs["sheep"], 1);
	}

	void TeleopInit()
	{
		SmartDashboard::PutBoolean("constant sheep", false);
		rumble_timer->Start();
	}

	void TeleopPeriodic()
	{
		float x = pilot->LeftX();
		float y = -pilot->LeftY();
		float rot = pilot->RightX();

		drive->MecanumDrive_Cartesian(x/2,y/2,rot/2);

		if(copilot->ButtonState(GamepadF310::BUTTON_START)){
			wing_fold->Set(WING_FOLD_SPEED);
		} else if(copilot->ButtonState(GamepadF310::BUTTON_BACK)) {
			wing_fold->Set( -WING_FOLD_SPEED );
		} else {
			wing_fold->Set(0.0);
		}


		wing_speed = clamp<float>(
				wing_speed + (WING_FLAP_ACCEL * (copilot->ButtonState(GamepadF310::BUTTON_B) ? 1 : -1)),
				0.0,
				WING_FLAP_SPEED
		);
		wing_flap->Set(wing_speed);

		// left trigger/button control head and jaw
		// right trigger/button control jaw only

		bool left_down = copilot->LeftTrigger() > 0.5;
		bool left_up = copilot->ButtonState(GamepadF310::BUTTON_LEFT_BUMPER);
		bool right_down = copilot->RightTrigger() >= 0.5f;
		bool right_up = copilot->ButtonState(GamepadF310::BUTTON_RIGHT_BUMPER);

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

		if (copilot->ButtonState(GamepadF310::BUTTON_A)) {
			eye_angle = ((1 - copilot->LeftX()) * 60) + 50;
		}
		eye->SetAngle(eye_angle);

		if (copilot->ButtonState(GamepadF310::BUTTON_X)) {
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

		if (copilot->ButtonState(GamepadF310::BUTTON_Y)) {
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

		if (pilot->ButtonState(GamepadF310::BUTTON_B)){
			pilot->RumbleLeft(1.0);
			pilot->RumbleRight(1.0);
		}
		else {
			pilot->RumbleLeft(0.0);
			pilot->RumbleRight(0.0);
		}


		CommonPeriodic();
	}

	void TestInit() {}

	void TestPeriodic() {}

	void CommonPeriodic() {
		for (auto it = sound_outputs.begin(); it != sound_outputs.end(); ++it) {
			setSound(it->second, false);
		}
		for (auto c = sound_choosers.begin(); c != sound_choosers.end(); ++c) {
			bool pressed = pilot->ButtonState(c->first);
			auto out = (DigitalOutput*)c->second->GetSelected();
			if (out){
				setSound(out, pressed);
				// Ensure that this sound is triggered if *any* buttons corresponding
				// to this sound are pressed. Without this, if buttons that are
				// checked later are not pressed and correspond to the same sound,
				// the sound will not be played.
				if (pressed)
					break;
			}
		}
	}
};

START_ROBOT_CLASS(Robot)
