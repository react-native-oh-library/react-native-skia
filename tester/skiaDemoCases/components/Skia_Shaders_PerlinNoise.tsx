import React from 'react';

import {
  Canvas,
  Fill,
  FractalNoise,
  Rect,
  Turbulence,
} from '@shopify/react-native-skia';
import {ScrollView, StyleSheet, View} from 'react-native';
import {Tester, TestCase} from '@rnoh/testerino';

export const FractalNoiseDemo = () => {
  return (
    <View style={styles.viewStyle}>
      <Canvas style={styles.canvasStyle}>
        <Fill color="white" />
        <Rect x={0} y={0} width={256} height={256}>
          <FractalNoise freqX={0.05} freqY={0.05} octaves={4} />
        </Rect>
      </Canvas>
    </View>
  );
};

export const TurbulenceNoiseDemo = () => {
  return (
    <View style={styles.viewStyle}>
      <Canvas style={styles.canvasStyle}>
        <Fill color="white" />
        <Rect x={0} y={0} width={256} height={256}>
          <Turbulence freqX={0.05} freqY={0.05} octaves={4} />
        </Rect>
      </Canvas>
    </View>
  );
};

export default function () {
  return (
    <Tester style={{flex: 1}}>
      <ScrollView>
        <TestCase itShould="case1: Fractal Perlin Noise Shader">
          <FractalNoiseDemo />
        </TestCase>
        <TestCase itShould="case2: Turbulence Perlin Noise Shader">
          <TurbulenceNoiseDemo />
        </TestCase>
      </ScrollView>
    </Tester>
  );
}

const styles = StyleSheet.create({
  viewStyle: {
    padding: 10,
    alignItems: 'center',
  },
  canvasStyle: {
    flex: 1,
    width: 256,
    height: 256,
  },
});
