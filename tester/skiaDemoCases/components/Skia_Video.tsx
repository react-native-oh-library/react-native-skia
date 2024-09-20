import React, {Component} from 'react';
import {Pressable, Text, View, useWindowDimensions} from 'react-native';

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
import {useSharedValue} from 'react-native-reanimated';

export const VideoExample = () => {
  const seek = useSharedValue<null | number>(null);
  // // Set this value to true to pause the video
  const paused = useSharedValue(false);
  const {width, height} = useWindowDimensions();
  const {currentFrame, currentTime} = useVideo(
    '../../assets/BigBuckBunny.mp4',
    {
      looping: true,
      seek,
      paused: paused,
      volume: 300.0,
    },
  );
  return (
    <Pressable
      style={{flex: 1}}
      onPress={() => {
        console.log('skia video---->press seek');
        seek.value = 1000;
      }}>
      <Canvas
        style={{flex: 1, width: width, height: 320, backgroundColor: 'cyan'}}>
        <Image
          image={currentFrame}
          x={0}
          y={0}
          width={width}
          height={320}
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
