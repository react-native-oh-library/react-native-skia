import React, {Component} from 'react';
import {Text, View} from 'react-native';

import {
  Box,
  BoxShadow,
  Canvas,
  rect,
  rrect,
  Oval,
  Shadow,
  Path,
  Skia,
  Group,
  Circle,
  vec,
  useFont,
  Fill,
  RoundedRect,
} from '@shopify/react-native-skia';

import {ScrollView} from 'react-native';
import {Tester, Filter, TestCase, TestSuite} from '@rnoh/testerino';

class DropShadowExample extends Component {
  render() {
    const width = 256;
    const height = 256;
    const r = width * 0.33;

    return (
      <View style={{padding: 10, alignItems: 'center'}}>
        <Canvas style={{width: 256, height: 256}}>
          <Fill color="lightblue" />
          <RoundedRect
            x={32}
            y={32}
            width={192}
            height={192}
            r={32}
            color="lightblue">
            <Shadow dx={12} dy={12} blur={25} color="#93b8c4" />
            <Shadow dx={-12} dy={-12} blur={25} color="#c7f8ff" />
          </RoundedRect>
        </Canvas>
      </View>
    );
  }
}

class InnerShadowExample extends Component {
  render() {
    const width = 256;
    const height = 256;
    const r = width * 0.33;

    return (
      <View style={{padding: 10, alignItems: 'center'}}>
        <Canvas style={{width: 256, height: 256}}>
          <Fill color="lightblue" />
          <RoundedRect
            x={32}
            y={32}
            width={192}
            height={192}
            r={32}
            color="lightblue">
            <Shadow dx={12} dy={12} blur={25} color="#93b8c4" inner />
            <Shadow dx={-12} dy={-12} blur={25} color="#c7f8ff" inner />
          </RoundedRect>
        </Canvas>
      </View>
    );
  }
}

export default function () {
  return (
    <Tester style={{flex: 1}}>
      <ScrollView>
        <TestCase itShould="case1: Drop Shadow">
          <DropShadowExample />
        </TestCase>

        <TestCase itShould="case2: Inner Shadow">
          <InnerShadowExample />
        </TestCase>
      </ScrollView>
    </Tester>
  );
}