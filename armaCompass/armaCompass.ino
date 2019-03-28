#include <Stepper.h>
#include <Encoder.h>

#define STEPS 32

Stepper _stepper(STEPS, 8, 10, 9, 11);
Encoder _encoder(2, 3);

int _currentDir = 0;
int _targetDir = 0;

int _speed = 32;
int _calibrationSpeed = 1;

int _val = 0;

float _stepsPerDegrees = 0.0f;
int _stepsPerRotation = 2038;

String _input = "";

int _encoderPinA = 2;
int _encoderPinB = 3;

int _encoderPosition = 0;
int _lastEncoderPosition = 0;

int _potentioPin = 1;
int _backlightPin = 6;


void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(0);

  _stepsPerDegrees = (float)_stepsPerRotation / 360.0f;
  
  _stepper.setSpeed(1024);
}

int calculateSteps(int targetDir)
{
  while(targetDir > _stepsPerRotation)
  {
    targetDir -= _stepsPerRotation;
  }
  while(targetDir < 0)
  {
    targetDir += _stepsPerRotation;
  }

  int stepsToTurn = targetDir - _currentDir;

  if(stepsToTurn > _stepsPerRotation / 2) // 350 - 10 = 340 | 340 > 180 | 350 - (10 + 360) = -20
  {
    stepsToTurn = targetDir - (_currentDir + _stepsPerRotation);
  }
  else if(stepsToTurn < -(_stepsPerRotation / 2)) // 10 - 350 = -340 | -340 < -180 | (10 + 360) - 350 = 20
  {
    stepsToTurn = (targetDir + _stepsPerRotation) - _currentDir;
  }

  return stepsToTurn * -1;
}

void readSerialInput()
{
  if(Serial.available() > 0)
  {
    String newInput = Serial.readString();

    _input += newInput;

    int idx = _input.indexOf(';');

    String headingString = "";

    if(idx > -1) // eof token was found
    {
      if(idx != _input.length()-1) // eof token is not at the very end
      {
        // get last complete message
        String tmp = _input.substring(0, _input.lastIndexOf(';'));
        int tmpIdx = tmp.indexOf(';');
        
        while(tmpIdx > -1)
        {
          tmp = tmp.substring(tmpIdx+1, tmp.length());
          tmpIdx = tmp.indexOf(';');
        }
        
        headingString = tmp;

        idx = _input.lastIndexOf(';');
        _input = _input.substring(idx+1, _input.length());
      }
      else
      {
        headingString = _input.substring(0, idx);
        _input = "";
      }
    }
    else // no complete message received yet
    {
      return;
    }

    // safety first...
    if(headingString.length() == 0)
    {
      return;
    }

    _targetDir = headingString.toInt() * _stepsPerDegrees;

    while(_targetDir > _stepsPerRotation)
    {
      _targetDir -= _stepsPerRotation;
    }

    // Serial.println(_targetDir);
  }
}

void loop()
{
  int potVal = analogRead(_potentioPin);
  potVal = 1023 - potVal;

  analogWrite(_backlightPin, potVal/4); // pot val ranges 0-1023, led expects between 0 and 255

  readSerialInput();

  _encoderPosition = _encoder.read();

  int _encoderDiff = _encoderPosition - _lastEncoderPosition;
  
  _lastEncoderPosition = _encoderPosition;

  int steps = calculateSteps(_targetDir);

  if(steps > _speed)
  {
    steps = _speed;
  }
  else if(steps < -1*_speed)
  {
    steps = -1*_speed;
  }

  if(steps != 0
    || _encoderDiff != 0)
  {
    _currentDir += steps * -1;

    steps += _encoderDiff * _calibrationSpeed;
    
    _stepper.step(steps);
  }
}
