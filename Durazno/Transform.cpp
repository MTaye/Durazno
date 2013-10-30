/*  Durazno - XInput to XInput Wrapper
 *  Copyright (C) 2012 KrossX
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Durazno.h"
#include "Transform.h"
#include "Settings.h"

#include <math.h>

extern _Settings settings[4];

inline double Linearity(double radius, short linearity)
{
	const double exp = linearity > 0 ? linearity +1 : 1.0/(-linearity+1);
	return pow(radius / 32768.0, exp) * 32768.0;
}

void TransformAnalog(SHORT &X, SHORT &Y, _Settings &set, bool leftStick)
{							
	// If input is dead, no need to check or do anything else
	if((X == 0) && (Y == 0)) return;

	double radius = sqrt((double)X*X + (double)Y*Y);
	double rX = X/radius, rY = Y/radius;
	
	if(set.deadzone > 0)
	{	
		double const max = 32768.0; // 40201 real max radius
		
		set.deadzone = max * set.deadzone; 	

		radius = radius <= set.deadzone ? 0 : (radius - set.deadzone) * max / (max - set.deadzone);

		if(set.linearity != 0) Linearity(radius, set.linearity);		
	}
	else if(set.linearity != 0) Linearity(radius, set.linearity);

	double dX = rX * radius;
	double dY = rY * radius;
			
	if(leftStick)
	{
		if(set.axisInverted[GP_AXIS_LX]) dX *= -1;
		if(set.axisInverted[GP_AXIS_LY]) dY *= -1;
	}
	else
	{
		if(set.axisInverted[GP_AXIS_RX]) dX *= -1;
		if(set.axisInverted[GP_AXIS_RY]) dY *= -1;
	}

	X = SHORT(dX < -32768 ? -32768 : dX > 32767 ? 32767 : dX);
	Y = SHORT(dY < -32768 ? -32768 : dY > 32767 ? 32767 : dY);
}

inline WORD Clamp(double input)
{
	unsigned int result = (unsigned int) input;
	result = result > 0xFFFF ? 0xFFFF : result;
	return (WORD)result;
}

void TransformGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	TransformAnalog(pState->Gamepad.sThumbLX, pState->Gamepad.sThumbLY, settings[dwUserIndex], true);
	TransformAnalog(pState->Gamepad.sThumbRX, pState->Gamepad.sThumbRY, settings[dwUserIndex], false);
}

void TransformSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
{
	pVibration->wLeftMotorSpeed = Clamp(pVibration->wLeftMotorSpeed * settings[dwUserIndex].rumble);
	pVibration->wRightMotorSpeed = Clamp(pVibration->wRightMotorSpeed * settings[dwUserIndex].rumble);
}