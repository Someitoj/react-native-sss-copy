import { Button, Modal, Alert, StyleSheet, Image, Platform, View, Pressable, Text } from 'react-native';

import { Collapsible } from '@/components/Collapsible';
import { ExternalLink } from '@/components/ExternalLink';
import ParallaxScrollView from '@/components/ParallaxScrollView';
import { ThemedText } from '@/components/ThemedText';
import { ThemedView } from '@/components/ThemedView';
import { IconSymbol } from '@/components/ui/IconSymbol';
import FontAwesome from '@expo/vector-icons/FontAwesome';

import { OpenCV, ColorConversionCodes } from 'react-native-fast-opencv';
import * as ImagePicker from 'expo-image-picker';
import {useState} from 'react';
import * as VisCam from 'react-native-vision-camera'
import { useRouter } from 'expo-router';

import { sss } from 'react-native-sss';

export default function TabTwoScreen() {
  const router = useRouter();
  const [image, setImage] = useState<string | null | undefined>(null);
  const [procImage, setProcImage] = useState<string | null | undefined>(null);
  const [procOldImage, setProcOldImage] = useState<string | null | undefined>(null);

  const pickImage = async () => {
    // No permissions request is necessary for launching the image library
    let result = await ImagePicker.launchImageLibraryAsync({
      mediaTypes: ['images', 'videos'],
      allowsEditing: true,
      aspect: [4, 3],
      quality: 1,
      base64: true
    })

    //console.log(result);

    if (!result.canceled) {
      setImage(result.assets[0].base64);
    }
  };
  
  //console.log("Test own library: 2 x 3 = " + multiply(2,3));

  

  const doOpenCV = () => {
    console.log('pressed Do OpenCV!!')
    
    if (image){
      
      const mat = OpenCV.base64ToMat(image);
      OpenCV.invoke('cvtColor', mat, mat, ColorConversionCodes.COLOR_BGR2GRAY);
      OpenCV.invoke('Canny', mat, mat, 50,100);
      const newImage = OpenCV.toJSValue(mat,"jpeg")
      //if(! newImage.base64.includes('data:image')){
      //  setProcImage("data:image/jpeg;base64," + newImage.base64);
      //}else{
        setProcOldImage(newImage.base64);
      //}
      
      //console.log(procImage)
      
      Alert.alert("Did OpenCV","You did OpenCV!");
    }else{
      Alert.alert("Noooo","You did not do OpenCV!");
    }
    OpenCV.clearBuffers();
    
    if (image){
      console.log("Start IMG");
      //console.log(image);
      try{
        var newImg = sss.test(image);
        console.log("Return IMG");
        //console.log(newImg);
        setProcImage(newImg.base64);
      } catch (error) {
        console.log(error);
      }
    } else {
      console.log('Did not do anything')
    }
     };

     const nav = () => {
      router.navigate("/visioncamera");
     };
  return (
    <ParallaxScrollView
      headerBackgroundColor={{ light: '#D0D0D0', dark: '#353636' }}
      headerImage={
        <IconSymbol
          size={310}
          color="#808080"
          name="chevron.left.forwardslash.chevron.right"
          style={styles.headerImage}
        />
      }>
        <ThemedView style={styles.titleContainer}>
            <ThemedText type="title">Test Tab</ThemedText>
        </ThemedView>
        
        <View style={styles.container}>
          <Button title="Take Picture" onPress={nav} />
          <Button title="Pick an image from camera roll" onPress={pickImage} />
          {image && <Image source={{ uri: "data:image/jpeg;base64," + image }} style={styles.image} />}
        </View>
        <View style={styles.container}>
          <Button title="Do OpenCV" onPress={doOpenCV}/>
          {procImage && <Image source={{ uri: "data:image/jpeg;base64," + procImage }} style={styles.image} />}
          {procOldImage && <Image source={{ uri: "data:image/jpeg;base64," + procOldImage }} style={styles.image} />}
        </View>

        
    </ParallaxScrollView>

  );
}

const styles = StyleSheet.create({
    headerImage: {
      color: '#808080',
      bottom: -90,
      left: -35,
      position: 'absolute',
    },
    titleContainer: {
      flexDirection: 'row',
      gap: 8,
    },
    buttonContainer: {
        width: 320,
        height: 68,
        marginHorizontal: 20,
        alignItems: 'center',
        justifyContent: 'center',
        padding: 3,
      },
      button: {
        borderRadius: 10,
        width: '100%',
        height: '100%',
        alignItems: 'center',
        justifyContent: 'center',
        flexDirection: 'row',
      },
      buttonIcon: {
        paddingRight: 8,
      },
      buttonLabel: {
        color: '#fff',
        fontSize: 16,
      },
      container: {
        flex: 1,
        alignItems: 'center',
        justifyContent: 'center',
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
        color: 'white',
        fontWeight: 'bold',
        textAlign: 'center',
      },
      modalText: {
        marginBottom: 15,
        textAlign: 'center',
      },
  });

