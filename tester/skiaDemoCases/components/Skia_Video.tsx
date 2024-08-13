import React, {Component} from 'react';
import {Pressable, Text, View, useWindowDimensions} from 'react-native';
import {useAssets} from 'expo-asset';

import {
  Box,
  BoxShadow,
  Canvas,
  rect,
  rrect,
  Fill,
  Image,
  useVideo,
} from '@shopify/react-native-skia';

import {ScrollView} from 'react-native';
import {Tester, TestCase} from '@rnoh/testerino';
// import {useSharedValue} from '@react-native-oh-tpl/react-native-reanimated';

export const VideoExample = () => {
  // const seek = useSharedValue<null | number>(null);
  // // Set this value to true to pause the video
  // const paused = useSharedValue(false);
  const {width, height} = useWindowDimensions();
  const {currentFrame, currentTime} = useVideo(
    '../../assets/BigBuckBunny.mp4',
    {
      looping: true,
    },
  );
  return (
    <Pressable style={{flex: 1, width: 256, height: 256}}>
      <Canvas style={{flex: 1, width: 256, height: 256}}>
        <Image
          image={currentFrame}
          x={0}
          y={0}
          width={width}
          height={height}
          fit="cover"
        />
      </Canvas>
    </Pressable>
  );
};

export default function () {
  return (
    <Tester style={{flex: 1}}>
      <ScrollView>
        <TestCase itShould="case1: VideoExample">
          <VideoExample />
        </TestCase>
      </ScrollView>
    </Tester>
  );
}
