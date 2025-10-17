import { Button, Modal, Alert, StyleSheet, Image, Platform, View, Pressable, Text } from 'react-native';

import { Collapsible } from '@/components/Collapsible';
import { ExternalLink } from '@/components/ExternalLink';
import ParallaxScrollView from '@/components/ParallaxScrollView';
import { ThemedText } from '@/components/ThemedText';
import { ThemedView } from '@/components/ThemedView';
import { IconSymbol } from '@/components/ui/IconSymbol';
import FontAwesome from '@expo/vector-icons/FontAwesome';

import Slider from '@react-native-community/slider';

import { OpenCV as cv, ColorConversionCodes, OpenCV } from 'react-native-fast-opencv';
import * as ImagePicker from 'expo-image-picker';
import React, {useEffect, useState} from 'react';
import {Camera, runAsync, runAtTargetFps, useCameraDevice, useCameraPermission, useFrameProcessor, getCameraDevice} from 'react-native-vision-camera'
import { router } from 'expo-router';
import { useResizePlugin } from 'vision-camera-resize-plugin';
import { useSharedValue, Worklets } from 'react-native-worklets-core';
import { sss } from 'react-native-sss';
import { transform } from '@babel/core';
import  Orientation from 'react-native-orientation-locker';

export default function TabTwoScreen() {
  Orientation.lockToPortrait();
  const [image, setImage] = useState<string | null | undefined>(null);
  const [oldImage, setOldImage] = useState<string | null | undefined>(null);
  const [fps, setFPS] = useState<number>(1);
  const [output, setOutput] = useState<string>("Output: ");

  const lastTime = useSharedValue(Date.now()/1000);
  const fpsStack = useSharedValue([0,0,0,0,0,0,0,0,0,0]);
  const counter = useSharedValue(0);

  const { hasPermission, requestPermission } = useCameraPermission();
  useEffect(() => {
    requestPermission();
  }, [requestPermission]);
  
  const device = useCameraDevice('back');
  if (device == null) { router.back();return;}
  const { resize } = useResizePlugin();

  const setLiveImage = Worklets.createRunOnJS((img: string) => {
    setImage(img);
  });
  const setLiveOldImage = Worklets.createRunOnJS((img: string) => {
    setOldImage(img);
  });
  const setLiveOutput = Worklets.createRunOnJS((out: string) => {
    setOutput("Output: " + out);
  });


 // FPS !!! https://react-native-vision-camera.com/docs/guides/formats#fps
  const frameProcessor = useFrameProcessor((frame) => {
    'worklet';
    /*
    const height = frame.height / 4;
    const width = frame.width / 4;
      
      const resized = resize(frame, {
        scale: {
          width: width,
          height: height,
        },
        pixelFormat: 'bgr',
        dataType: 'uint8',
      });
      
      const mat = cv.frameBufferToMat(height, width, 3, resized);

      cv.invoke('cvtColor', mat, mat, ColorConversionCodes.COLOR_BGR2GRAY);
      cv.invoke('Canny', mat, mat, 50,100);
      const newImage = cv.toJSValue(mat,"jpeg")

      setLiveOldImage(newImage.base64);
    
      cv.clearBuffers(); // IMPORTANT! At the end.
    */
   //runAsync(frame, () => {
    runAtTargetFps(1, () => {
      'worklet';
      
      const height = frame.height / 4;
      const width = frame.width / 4;

      const resized = resize(frame, {
        scale: {
          width: frame.width,
          height: frame.height,
        },
        pixelFormat: 'bgr',
        dataType: 'uint8',
        
      });
      
      
      if(false){
        const mat = cv.frameBufferToMat(frame.height, frame.width, 3, resized);
        
        cv.invoke('cvtColor', mat, mat, ColorConversionCodes.COLOR_BGR2GRAY);
        cv.invoke('Canny', mat, mat, 50,100);
        const newImage = cv.toJSValue(mat,"jpeg");
  
        setLiveOldImage(newImage.base64);
        
        cv.clearBuffers(); // IMPORTANT! At the end.
      }
      
     /*
      var iii = sss.testFrame(frame.height, frame.width, 3, resized);
      if(iii.base64){
        setLiveImage(iii.base64);
      }
      */
      var out = sss.processFrame(frame.height, frame.width, 3, resized).output;
      console.log("Output:",out );
      //console.log("Frame Orient:",frame.orientation);
      //console.log("DEVICE ORI", device.sensorOrientation);
      
      
      //console.log("Device Orient:", );
      //setLiveOutput(out);
      

      var time = Date.now() / 1000;
      fpsStack.value[counter.value] = Math.floor(100 / (time - lastTime.value))/100;
      lastTime.value = time;
      //console.log(fpsStack.value);
      if(counter.value == 4 || counter.value >= 9){
        var total = 0;
        for(var i = 0; i < fpsStack.value.length; i++) {
            total += fpsStack.value[i];
        }
        var avg = total / fpsStack.value.length;
        console.log("FPS set at: ",fps," Current FPS: ", avg);
      }
      if(counter.value >= 9){
        counter.value = 0;
      }else{
        counter.value = counter.value + 1;
      }
    });

    
  },[setLiveImage,setLiveOldImage, setLiveOutput, fps, lastTime, counter, fpsStack]);

  return (
    <View style={styles.container}>
      <View style={styles.containerTop}>
        <Text style={styles.textStyle}>{output}</Text>
        <View style={styles.containerImages}>
          {image && <Image source={{ uri: "data:image/jpeg;base64," + image }} style={styles.image} />}
          {oldImage && <Image source={{ uri: "data:image/jpeg;base64," + oldImage }} style={styles.image} />}
        </View>
      </View>
      <View style={styles.containerBottom}>
        <Camera
        style={StyleSheet.absoluteFill}
        device={device}
        isActive={true}
        frameProcessor={frameProcessor}
        outputOrientation='device'
        />
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
      container: {
        flex: 1,
        alignItems: 'center',
        justifyContent: 'center',
        flexDirection: 'column',
      },
      containerBottom: {
        flex: 1,
        alignItems: 'center',
        alignSelf:'stretch', 
        justifyContent: 'center',
        flexDirection: 'column',
        backgroundColor: 'grey',
      },
      containerTop: {
        flex: 1,
        alignItems: 'center',
        justifyContent:'space-around',
        flexDirection: 'column',
        height: 250,
        backgroundColor: 'gray'
      },
      containerImages: {
        flex: 1,
        alignItems: 'center',
        justifyContent: 'center',
        flexDirection: 'row',
        height: 210,
      },
      image: {
        width: 200,
        height: 200,
        
      },
      centeredView: {
        flex: 1,
        justifyContent: 'center',
        alignItems: 'center',
      },
      modalView: {
        margin: 20,
        backgroundColor: 'white',
        borderRadius: 20,
        padding: 35,
        alignItems: 'center',
        shadowColor: '#000',
        shadowOffset: {
          width: 0,
          height: 2,
        },
        shadowOpacity: 0.25,
        shadowRadius: 4,
        elevation: 5,
      },
      buttonOpen: {
        backgroundColor: '#F194FF',
      },
      buttonClose: {
        backgroundColor: '#2196F3',
      },
      textStyle: {
        color: 'black',
        fontWeight: 'bold',
        textAlign: 'center',
      },
      modalText: {
        marginBottom: 15,
        textAlign: 'center',
      },
  });

