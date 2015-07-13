/*
 * GamepadF310.h
 *
 *  Created on: Jan 5, 2015
 *      Author: ratpack
 */

#include "WPILib.h"

#ifndef UTIL_GAMEPADF310_H_
#define UTIL_GAMEPADF310_H_

class GamepadF310 {
public:
	GamepadF310(int port);
	virtual ~GamepadF310();
	float LeftX();
	float LeftY();
	float LeftTrigger();
	float RightTrigger();
	float RightX();
	float RightY();
	float DPadX(); //only returns +1, 0, or -1
	float DPadY(); //only returns +1, 0, or -1

	void LeftRumble(float rumbleness); //doesn't actually do anything on our controllers :(
	void RightRumble(float rumbleness); //ditto

	bool Button(int buttonNum);
	bool LeftStickPress();
	bool RightStickPress();
	bool LeftBumper();
	bool RightBumper();

	//the gamepad buttons
	static const int A_Button = 1;
	static const int B_Button = 2;
	static const int X_Button = 3;
	static const int Y_Button = 4;
	static const int BACK_BUTTON = 7;
	static const int START_BUTTON = 8;


protected:
	static const int LEFT_X_AXIS_NUM = 0;
	static const int LEFT_Y_AXIS_NUM = 1;
	static const int LEFT_TRIGGER_NUM = 2;
	static const int RIGHT_TRIGGER_NUM = 3;
	static const int RIGHT_X_AXIS_NUM = 4;
	static const int RIGHT_Y_AXIS_NUM = 5;

	Joystick * joystick;
};

#endif /* SRC_GAMEPADF310_H_ */
