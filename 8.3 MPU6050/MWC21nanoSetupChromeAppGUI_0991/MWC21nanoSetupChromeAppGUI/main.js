/*
NanoWii Setup GUI

Dieses Projekt ist lizensiert als Inhalt der Creative Commons Namensnennung-Nicht-kommerziell 3.0 
Unported-Lizenz. Um eine Kopie der Lizenz zu sehen, besuchen Sie http://creativecommons.org/licenses/by-nc/3.0/.
(cc-by-nc)

Autor: Felix Niessen (flyduino.net)

*/

var baud = 115200;



var SeTBuf = [];
var toSeTbuf = 0;
var SeTBufC  = 0;



var readSerial = 1;
var connId = [];


var F3D                    = 0; // ESC's are set to 3D Mode
var MIDDLEDEADBAND = 40; //nur für 3D

var sOneShot            = 0; //0=normaler betrieb (4xxhz) 1 ist oneshot 125 

var copterType          =3; //0=Bi,1=Tri,2=QUADP,3=QUADX,4=Y4,5=Y6,6=H6P,7=H6X,8=Vtail4

var RxType               = 0; //0StandardRX,1sat1024,2sat2048,3PPMGrSp,4PPMRobHiFu,5PPMHiSanOthers

var MINTHROTTLE     = 1150;
var MAXTHROTTLE   = 2000;
var MINCOMMAND    = 1000;
var MIDRC              = 1500;
var MINCHECK         = 1100;
var MAXCHECK       = 1900;

var BILeftMiddle      = 1500;
var BIRightMiddle    = 1500;
var TriYawMiddle     = 1500;
var YAW_DIRECTION = 0;
var BiLeftDir           = 0;
var BiRightDir         = 0;
var DigiServo          = 0;

var ArmRoll             = 0;  // arm und disarm über roll statt yaw

var MPU6050_DLPF_CFG = 3; //0=256(aus),1=188,2=98,3=42,4=20,5=10,6=5 
var s3DMIDDLE        = 1500;


var readB = 0;
var writeB = 0;

function openPort(ports){
	readB = document.getElementById('read');
	writeB = document.getElementById('write');
	
	readB.onclick = function(){readIt = 1;Write('_B');};
	writeB.onclick = function(){WriteSettings();};
	
	
	document.getElementById('close').innerHTML = '';
	document.getElementById('comsel').innerHTML = '';
	var portwahl = document.createElement('select');
	portwahl.onchange = function(){
		openSerial(this.value);
	};
	portwahl.id = 'comselin';
	var portOpt = document.createElement('option');
	portOpt.value = '';
	portOpt.innerHTML = 'Select com Port';
	portwahl.appendChild(portOpt);
	for(var i in ports){
		var portOpt = document.createElement('option');
		portOpt.value = ports[i].path;
		portOpt.innerHTML = ports[i].path;
		
		portwahl.appendChild(portOpt);
	}
	document.getElementById('comsel').appendChild(portwahl);
	setConfig();
	
	
	var refresh = document.createElement('input');
	refresh.type = 'button';
	refresh.value = 'Refresh';
	refresh.style.fontSize = '11px';
	refresh.style.width = '80px';
	refresh.onclick = function(){
		chrome.serial.getDevices(function(ports){
			openPort(ports);
		});
	}
	document.getElementById('close').appendChild(refresh);
	
	document.getElementById('rcmiddle').onchange=function(){MIDRC=this.value; setConfig();};
	document.getElementById('rcmax').onchange=function(){MAXCHECK=this.value; setConfig();};
	document.getElementById('rcmin').onchange=function(){MINCHECK=this.value; setConfig();};
	
	document.getElementById('escmin').onchange=function(){MINCOMMAND=this.value; setConfig();};
	document.getElementById('escidle').onchange=function(){MINTHROTTLE=this.value; setConfig();};
	document.getElementById('escmax').onchange=function(){MAXTHROTTLE=this.value; setConfig();};
	
	document.getElementById('oneshot125').onchange=function(){sOneShot=this.value; setConfig();};
	document.getElementById('3desc').onchange=function(){F3D=this.value; setConfig();};
	document.getElementById('3dmid').onchange=function(){s3DMIDDLE=this.value; setConfig();};
	
	document.getElementById('3ddb').onchange=function(){MIDDLEDEADBAND=this.value; setConfig();};
	
	
	document.getElementById('MPUlpf').onchange=function(){MPU6050_DLPF_CFG=this.value; setConfig();};
	document.getElementById('armstick').onchange=function(){ArmRoll=this.value; setConfig();};
	document.getElementById('Spwm').onchange=function(){DigiServo=this.value; setConfig();};
	
	document.getElementById('yawd').onchange=function(){YAW_DIRECTION=this.value; setConfig();};
	document.getElementById('tymid').onchange=function(){TriYawMiddle=this.value; setConfig();};
	document.getElementById('bild').onchange=function(){BiLeftDir=this.value; setConfig();};
	
	document.getElementById('bilmid').onchange=function(){BILeftMiddle=this.value; setConfig();};
	document.getElementById('bird').onchange=function(){BiRightDir=this.value; setConfig();};
	document.getElementById('birmid').onchange=function(){BIRightMiddle=this.value; setConfig();};
	
	
}

initDone = 0;

function openSerial(portVal){
	document.getElementById('close').innerHTML = '';
	if(portVal == '') return;
	document.getElementById('comsel').removeChild(document.getElementById('comselin'));
        chrome.serial.connect(portVal, {bitrate: baud}, function (cInfo) {serOpen(cInfo);});
	if(initDone == 0){
		chrome.serial.onReceive.addListener(pullBuf);
		initDone = 1;
	}	
	
	var closeComm = document.createElement('input');
	closeComm.type = 'button';
	closeComm.value = 'Close COM';
	closeComm.style.fontSize = '11px';
	closeComm.style.width = '80px';
	closeComm.onclick = function(){
		readB.setAttribute("disabled","disabled");
		writeB.setAttribute("disabled","disabled");
		chrome.serial.send(connId.connectionId, str2ab('XXXX'), closen);
		//chrome.serial.flush(connId.connectionId, onFlush);
	}
	document.getElementById('close').appendChild(closeComm);
	saledPort  = portVal;
	var opening = document.createElement('h2');
	opening.innerHTML ="selecting port: "+portVal;
	opening.id="opening";
	readB.removeAttribute("disabled");
	writeB.removeAttribute("disabled");

}

function serOpen(cInfo){
	connId = cInfo;
	if(document.getElementById('opening')){
		document.body.removeChild(document.getElementById('opening'));
	}
	document.getElementById('comsel').innerHTML = saledPort;
	
	initGUI();
}


function Write(data){
	chrome.serial.send(connId.connectionId, str2ab(data), onWrite);
	//chrome.serial.flush(connId.connectionId, onFlush);
}
function onWrite(){

}
function closen(){
	readB.setAttribute("disabled","disabled");
	writeB.setAttribute("disabled","disabled");
	chrome.serial.disconnect(connId.connectionId, function(){chrome.serial.getDevices(function(ports){
		openPort(ports);
	});});
}
function onFlush(){
	
}

function pullBuf(info){
	checkComm(info);
}


var canvasCont = 0;
//_______________________________________________________________________//
var readIt = 0;
function initGUI(){
	readIt = 1;
	Write('#');
	Write('_B');
}

var recived = 0;
var TMP_buff = [];
function checkComm(readInfo){
	if (readInfo.data.byteLength > 0) {
		var data = new Uint8Array(readInfo.data);
		
		
		for(var i = 0; i < data.length; i++){
			if(data[i] == 66 && toSeTbuf == 0){
				TMP_buff = [];
				toSeTbuf = 1;
				SeTBufC  = 0;
				continue;
			}
			if(toSeTbuf){
				TMP_buff[SeTBufC] = data[i];
				SeTBufC++;
			}
			if(SeTBufC > 30 && toSeTbuf){
				toSeTbuf = 0;
				SeTBuf = TMP_buff;
				checkSetupData();
				readIt = 0;
			}	
		}
	}
};



function checkSetupData(){
	
      F3D            = SeTBuf[0]; // ESC's are set to 3D Mode
      MIDDLEDEADBAND = SeTBuf[1]; //nur für 3D
      
      sOneShot       = SeTBuf[2]; //0=normaler betrieb (4xxhz) 1 ist oneshot 125 
      
      copterType     = SeTBuf[3]; //0=Bi,1=Tri,2=QUADP,3=QUADX,4=Y4,5=Y6,6=H6P,7=H6X,8=Vtail4
      
      RxType         = SeTBuf[4]; //0StandardRX,1sat1024,2sat2048,3PPMGrSp,4PPMRobHiFu,5PPMHiSanOthers
      
      MINTHROTTLE   = (SeTBuf[6]<<8) | SeTBuf[5];
      MAXTHROTTLE   = (SeTBuf[8]<<8) | SeTBuf[7];
      MINCOMMAND    = (SeTBuf[10]<<8) | SeTBuf[9];
      MIDRC         = (SeTBuf[12]<<8) | SeTBuf[11];
      MINCHECK      = (SeTBuf[14]<<8) | SeTBuf[13];
      MAXCHECK      = (SeTBuf[16]<<8) | SeTBuf[15];
      
      BILeftMiddle  = (SeTBuf[18]<<8) | SeTBuf[17];
      BIRightMiddle = (SeTBuf[20]<<8) | SeTBuf[19];
      TriYawMiddle  = (SeTBuf[22]<<8) | SeTBuf[21];
      YAW_DIRECTION = SeTBuf[23];
      BiLeftDir = SeTBuf[24];
      BiRightDir = SeTBuf[25];
      DigiServo = SeTBuf[26]
      ArmRoll       = SeTBuf[27];  // arm und disarm über roll statt yaw
      MPU6050_DLPF_CFG = SeTBuf[28]; //0=256(aus),1=188,2=98,3=42,4=20,5=10,6=5 
      s3DMIDDLE = (SeTBuf[30]<<8) | SeTBuf[29];
      setConfig();
}

var toWrite = [];
var WriteCount = 0;

function WriteSettings(){
/*
      if(WriteCount == 0){
	      toWrite.push('A');
		
	      toWrite.push(F3D);
	      toWrite.push(MIDDLEDEADBAND);
	      
	      toWrite.push(sOneShot);
	      
	      toWrite.push(copterType);
	      
	      toWrite.push(RxType); 
	      
	      toWrite.push(MINTHROTTLE>>8);toWrite.push(MINTHROTTLE&0xff);
	      toWrite.push(MAXTHROTTLE>>8);toWrite.push(MAXTHROTTLE&0xff);
	      toWrite.push(MINCOMMAND>>8);toWrite.push(MINCOMMAND&0xff); 
	      toWrite.push(MIDRC>>8);toWrite.push(MIDRC&0xff);
	      toWrite.push(MINCHECK>>8);toWrite.push(MINCHECK&0xff); 
	      toWrite.push(MAXCHECK>>8);toWrite.push(MAXCHECK&0xff);
	      toWrite.push(BILeftMiddle>>8);toWrite.push(BILeftMiddle&0xff);
	      toWrite.push(BIRightMiddle>>8);toWrite.push(BIRightMiddle&0xff);
	      toWrite.push(TriYawMiddle>>8);toWrite.push(TriYawMiddle&0xff);
	      toWrite.push(YAW_DIRECTION);
	      toWrite.push(BiLeftDir);
	      toWrite.push(BiRightDir);
	      toWrite.push(DigiServo);
	      
	      toWrite.push(ArmRoll);
	      
	      toWrite.push(MPU6050_DLPF_CFG);
	      toWrite.push(s3DMIDDLE>>8);toWrite.push(s3DMIDDLE&0xff);
      }
      if(WriteCount < 31){
	        if(WriteCount == 0){
			chrome.serial.write(connId.connectionId, str2ab(toWrite[0]), WriteSettings);
		}else{
			chrome.serial.write(connId.connectionId, str2ab(getA2sign(toWrite[WriteCount])), WriteSettings);
		}
		chrome.serial.flush(connId.connectionId, onFlush);		      
	        WriteCount++;
      }else{
		chrome.serial.write(connId.connectionId, str2ab(getA2sign(toWrite[31])), closen);
		chrome.serial.flush(connId.connectionId, onFlush);	
                toWrite = [];
		WriteCount = 0;	  	      
      }
*/

      var toWrite = 'A';
	
      toWrite += getA2sign(F3D);
      toWrite += getA2sign(MIDDLEDEADBAND);
      
      toWrite += getA2sign(sOneShot);
      
      toWrite += getA2sign(copterType);
      
      toWrite += getA2sign(RxType); 
      
      toWrite += getA2sign(MINTHROTTLE>>8)+''+getA2sign(MINTHROTTLE&0xff);
      toWrite += getA2sign(MAXTHROTTLE>>8)+''+getA2sign(MAXTHROTTLE&0xff);
      toWrite += getA2sign(MINCOMMAND>>8)+''+getA2sign(MINCOMMAND&0xff); 
      toWrite += getA2sign(MIDRC>>8)+''+getA2sign(MIDRC&0xff);
      toWrite += getA2sign(MINCHECK>>8)+''+getA2sign(MINCHECK&0xff); 
      toWrite += getA2sign(MAXCHECK>>8)+''+getA2sign(MAXCHECK&0xff);
      toWrite += getA2sign(BILeftMiddle>>8)+''+getA2sign(BILeftMiddle&0xff);
      toWrite += getA2sign(BIRightMiddle>>8)+''+getA2sign(BIRightMiddle&0xff);
      toWrite += getA2sign(TriYawMiddle>>8)+''+getA2sign(TriYawMiddle&0xff);
      toWrite += getA2sign(YAW_DIRECTION);
      toWrite += getA2sign(BiLeftDir);
      toWrite += getA2sign(BiRightDir);
      toWrite += getA2sign(DigiServo);
      
      toWrite += getA2sign(ArmRoll);
      
      toWrite += getA2sign(MPU6050_DLPF_CFG);
      toWrite += getA2sign(s3DMIDDLE>>8)+''+getA2sign(s3DMIDDLE&0xff);
      
	chrome.serial.send(connId.connectionId, str2ab( toWrite), closen);
	//chrome.serial.flush(connId.connectionId, onFlush);

}

function getA2sign(numb){
	return String.fromCharCode(numb);
}


function setConfig(){
	//copter type
	for(var i = 0; i< 9; i++){
		if(i == copterType){
			document.getElementById('ct'+i).style.opacity = '1';
			document.getElementById('ct'+i).onmouseover = function(){
				return;
			}
			document.getElementById('ct'+i).onmouseout = function(){
				return;
			}
			document.getElementById('ct'+i).onclick = function(){
				return;
			}
		}else{
			document.getElementById('ct'+i).style.opacity = '0.3';
			document.getElementById('ct'+i).onmouseover = function(){
				this.style.opacity = '1';
			}
			document.getElementById('ct'+i).onmouseout = function(){
				this.style.opacity = '0.3';
			}
			document.getElementById('ct'+i).onclick = function(){
				copterType = parseInt(this.id.replace(/ct/,''));
				setConfig();
			}
		}
	}
	// RX types
	for(var i = 0; i< 6; i++){
		if(i == RxType){
			document.getElementById('rxt'+i).style.opacity = '1';
			document.getElementById('rxt'+i).onmouseover = function(){
				return;
			}
			document.getElementById('rxt'+i).onmouseout = function(){
				return;
			}
			document.getElementById('rxt'+i).onclick = function(){
				return;
			}
		}else{
			document.getElementById('rxt'+i).style.opacity = '0.3';
			document.getElementById('rxt'+i).onmouseover = function(){
				this.style.opacity = '1';
			}
			document.getElementById('rxt'+i).onmouseout = function(){
				this.style.opacity = '0.3';
			}
			document.getElementById('rxt'+i).onclick = function(){
				RxType = parseInt(this.id.replace(/rxt/,''));
				setConfig();
			}
		}
	}
	if(MIDRC > 1700) MIDRC = 1700;
	if(MIDRC < 1300) MIDRC = 1300;
	document.getElementById('rcmiddle').value = MIDRC;
	if(MAXCHECK > 2000) MAXCHECK = 2000;
	if(MAXCHECK < 1700) MAXCHECK = 1700;
	document.getElementById('rcmax').value = MAXCHECK;
	if(MINCHECK > 1300) MINCHECK = 1300;
	if(MINCHECK < 1000) MINCHECK = 1000;
	document.getElementById('rcmin').value = MINCHECK;
	
	// ESC setup
	
	if(MINCOMMAND > 1200) MINCOMMAND = 1200;
	if(MINCOMMAND < 900) MINCOMMAND = 900;
	document.getElementById('escmin').value = MINCOMMAND;
	if(MINTHROTTLE > 1400) MINTHROTTLE = 1400;
	if(MINTHROTTLE < 900) MINTHROTTLE = 900;
	document.getElementById('escidle').value = MINTHROTTLE;
	if(MAXTHROTTLE > 2000) MAXTHROTTLE = 2000;
	if(MAXTHROTTLE < 1700) MAXTHROTTLE = 1700;
	document.getElementById('escmax').value = MAXTHROTTLE;
	
        for(var i = 0; i < 2; i++){
		if(i == sOneShot){
			document.getElementById('os'+i).selected = 'selected';
		}else{
			if(document.getElementById('os'+i).selected == 'selected'){
				document.getElementById('os'+i).removeAttribute("selected");
			}
		}
		
	}
	for(var i = 0; i < 2; i++){
		if(i == F3D){
			document.getElementById('3d'+i).selected = 'selected';
		}else{
			if(document.getElementById('3d'+i).selected == 'selected'){
				document.getElementById('3d'+i).removeAttribute("selected");
			}
		}
		
	}
	
	if(s3DMIDDLE > 1700) s3DMIDDLE = 1700;
	if(s3DMIDDLE < 1300) s3DMIDDLE = 1300;
	document.getElementById('3dmid').value = s3DMIDDLE;
	if(MIDDLEDEADBAND > 150) MIDDLEDEADBAND = 150;
	if(MIDDLEDEADBAND < 26) MIDDLEDEADBAND = 26;	
	document.getElementById('3ddb').value = MIDDLEDEADBAND;
	
	
	// other stuff
        for(var i = 0; i < 7; i++){
		if(i == MPU6050_DLPF_CFG){
			document.getElementById('lpf'+i).selected = 'selected';
		}else{
			if(document.getElementById('lpf'+i).selected == 'selected'){
				document.getElementById('lpf'+i).removeAttribute("selected");
			}
		}
	}	
	
        for(var i = 0; i < 2; i++){
		if(i == ArmRoll){
			document.getElementById('as'+i).selected = 'selected';
		}else{
			if(document.getElementById('as'+i).selected == 'selected'){
				document.getElementById('as'+i).removeAttribute("selected");
			}
		}
	}	
	
        for(var i = 0; i < 2; i++){
		if(i == DigiServo){
			document.getElementById('ds'+i).selected = 'selected';
		}else{
			if(document.getElementById('ds'+i).selected == 'selected'){
				document.getElementById('ds'+i).removeAttribute("selected");
			}
		}
	}
	
	for(var i = 0; i < 2; i++){
		if(i == YAW_DIRECTION){
			document.getElementById('ya'+i).selected = 'selected';
		}else{
			if(document.getElementById('ya'+i).selected == 'selected'){
				document.getElementById('ya'+i).removeAttribute("selected");
			}
		}
	}
	if(TriYawMiddle > 1700) TriYawMiddle = 1700;
	if(TriYawMiddle < 1300) TriYawMiddle = 1300;
	document.getElementById('tymid').value = TriYawMiddle;
	
	for(var i = 0; i < 2; i++){
		if(i == BiLeftDir){
			document.getElementById('bil'+i).selected = 'selected';
		}else{
			if(document.getElementById('bil'+i).selected == 'selected'){
				document.getElementById('bil'+i).removeAttribute("selected");
			}
		}
	}
	if(BILeftMiddle > 1700) BILeftMiddle = 1700;
	if(BILeftMiddle < 1300) BILeftMiddle = 1300;
	document.getElementById('bilmid').value = BILeftMiddle;
	
	for(var i = 0; i < 2; i++){
		if(i == BiRightDir){
			document.getElementById('bir'+i).selected = 'selected';
		}else{
			if(document.getElementById('bir'+i).selected == 'selected'){
				document.getElementById('bir'+i).removeAttribute("selected");
			}
		}
	}
	if(BIRightMiddle > 1700) BIRightMiddle= 1700;
	if(BIRightMiddle < 1300) BIRightMiddle = 1300;
	document.getElementById('birmid').value = BIRightMiddle;
	
	
	
	
}







//_______________________________________________________________________//









//_______________________________________________________________________//

onload = function(){
	chrome.serial.getDevices(function(ports){
		openPort(ports);
	});
}

onclose = function(){
	chrome.serial.disconnect(connId.connectionId, function(){});
	
};

//part of chrome serial lib
/**
Copyright 2012 Google Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Author: Renato Mangini (mangini@chromium.org)
Author: Luis Leao (luisleao@gmail.com)
**/

var str2ab = function (str) {
        var buf = new ArrayBuffer(str.length);
        var bufView = new Uint8Array(buf);
        for (var i = 0; i < str.length; i++) {
            bufView[i] = str.charCodeAt(i);
        }
        return buf;
};










