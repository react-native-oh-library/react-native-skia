import React from 'react';
import {StyleSheet, View} from 'react-native';

import {
  Canvas,
  useImage,
  Image,
  Offset,
  Fill,
  Morphology,
  Text,
  useFont,
} from '@shopify/react-native-skia';

import {ScrollView} from 'react-native';
import {Tester, Filter, TestCase, TestSuite} from '@rnoh/testerino';

const MorphologyDemo = () => {
  const font = useFont(require('../../assets/fonts/Pacifico-Regular.ttf'), 24);
  return (
    <View style={styles.viewStyle}>
      <Canvas style={styles.canvasStyle}>
        <Text text="Hello World" x={32} y={32} font={font} />
        <Text text="Hello World" x={32} y={64} font={font}>
          <Morphology radius={1} />
        </Text>
        <Text text="Hello World" x={32} y={96} font={font}>
          <Morphology radius={0.3} operator="erode" />
        </Text>
      </Canvas>
    </View>
  );
};

export default function () {
  return (
    <Tester style={{flex: 1}}>
      <ScrollView>
        <TestCase itShould="case1: The morphology image filter is identical to its SVG counterpart. It is used to erode or dilate the input image. Its usefulness lies primarily in fattening or thinning effects.">
          <MorphologyDemo />
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