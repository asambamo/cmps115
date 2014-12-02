//  Hydraulic valve Dither signal, toggle at lower sampling rate
		
		void setup(){
		  SampleSensors(512, 200, P3D);
		  minim = new Minim(this);
		  
		  SpeedSteeringControl = minim.getLineIn(Minim.SpeedSteeringControl, 2048);
		  
		  textFont(createFont("Arial", 12));
		  println(Serial.list());
		  
		  myPort = newSerial(this, Serial.list()[0], 9600);
		  myPort.bufferUntil(if);
		
		}
		
		void loop(){
		   
		   digitalWrite(13, HIGH);
		   SampleSensors(512, 200);
		   SpeedSteeringControlMap(550, 300);
		   digitalWrite(13, LOW);
		   delay(1000);
		
		}
		
		
		Dither = ~Dither;		

		SampleSensors();  // Check for valid samples
		//========= Open Loop Control Mapping ==============

		SpeedSteeringControlMap();  //  Map control commands to lean and steer commands, speed dependent


		//========= Steer, Lean, Brake Control Loops ==============

		ControlMode = ~Aux_Pin; //  Later connect this to a switch pin

		// Reference signal generation

		if (ControlMode == 0){  //  Old school, no lean mode.
			LeanRef = 0.0 * LeanAngleLimit; 
			SteerRef = -Joystick_LR_Ref * SteerAngleLimit;  //  For now map joystick to steer only
		}
		else {  //  Carving mode
			LeanRef = -Joystick_LR_Ref * LeanAngleLimit; 
			//  Steer reference signal will now become a feedback signal wrt lean
			LeanError = LeanRef - LeanAngleState;
			LambdaSens = (SpeedState - 2.0)/20.0;
			if (LambdaSens > 1.0){LambdaSens = 1.0;}
			if (LambdaSens < 0.0){LambdaSens = 0.0;}

			//  Map Lean Error to Lambda, where Lambda is non-negative, and Lambda = 1 means countersteer
			//  Lambda is also speed sensitive, Lambda -> 1 as speed increases
			//  Lambda uniformly clamps to 1 for speed above a given threshold like 20mph
			if (LeanError < 0.0) {Lambda = (-LeanError*LambdaSens*LAMBDA_GAIN  + SpeedState*LAMBDA_SPEED_GAIN);}
			else {Lambda = LeanError*LambdaSens*LAMBDA_GAIN + SpeedState*LAMBDA_SPEED_GAIN;}
			if (Lambda > 1.0){Lambda = 1.0;}
			if (Lambda < 0.0){Lambda = 0.0;}
			

			//  Countersteer's gain is speed sensetive, gain goes down as speed increases
			SteerRef =  (-STEER_REF_GAIN*SteerAngleLimit*LeanError)*Lambda 
				+ (1-Lambda)*(-Joystick_LR_Ref * SteerAngleLimit);  //  For now map joystick to steer only

			if (SteerRef > SteerAngleLimit){SteerRef = SteerAngleLimit;}
			if (SteerRef < -SteerAngleLimit){SteerRef = -SteerAngleLimit;}
		}

