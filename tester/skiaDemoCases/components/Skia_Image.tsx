import React, {Component} from 'react';

import {
  Box,
  BoxShadow,
  Canvas,
  rect,
  rrect,
  Fill,
  useImage,
  Image,
  ImageSVG,
  useSVG,
} from '@shopify/react-native-skia';

import {ScrollView, View} from 'react-native';
import {Tester, TestCase} from '@rnoh/testerino';

const ImageDemo = () => {
  const image = useImage(require('../../assets/placeholder2000x2000.jpg'));
  return (
    <View style={{padding: 10, alignItems: 'center'}}>
      <Canvas style={{flex: 1, width: 256, height: 256}}>
        <Image
          image={image}
          fit="contain"
          x={0}
          y={0}
          width={256}
          height={256}
        />
      </Canvas>
    </View>
  );
};

const GifImageDemo = () => {
  const image = useImage(require('../../assets/gifdemo.gif'));
  return (
    <View style={{padding: 10, alignItems: 'center'}}>
      <Canvas style={{flex: 1, width: 256, height: 256}}>
        <Image
          image={image}
          fit="contain"
          x={0}
          y={0}
          width={256}
          height={256}
        />
      </Canvas>
    </View>
  );
};

const SVGImageDemo = () => {
  const svg = useSVG(require('../../assets/tiger.svg'));
  return (
    <View style={{padding: 10, alignItems: 'center'}}>
      <Canvas style={{flex: 1, width: 256, height: 256}}>
        {svg && <ImageSVG svg={svg} width={256} height={256} />}
      </Canvas>
    </View>
  );
};

export default function () {
  return (
    <Tester style={{flex: 1}}>
      <ScrollView>
        <TestCase itShould="case1: Image">
          <ImageDemo />
        </TestCase>
        <TestCase itShould="case1: Gif Image Demo">
          <GifImageDemo />
        </TestCase>
        <TestCase itShould="case1: SVG Image Demo">
          <SVGImageDemo />
        </TestCase>
      </ScrollView>
    </Tester>
  );
}
