import React from 'react';
import {StyleSheet, View} from 'react-native';

import {
  Canvas,
  useImage,
  Image,
  Offset,
  Fill,
} from '@shopify/react-native-skia';

import {ScrollView} from 'react-native';
import {Tester, Filter, TestCase, TestSuite} from '@rnoh/testerino';

const OffsetDemo = () => {
  const image = useImage(require('../../assets/oslo.jpg'));
  if (!image) {
    return null;
  }
  return (
    <View style={styles.viewStyle}>
      <Canvas style={styles.canvasStyle}>
        <Fill color="lightblue" />
        <Image image={image} x={0} y={0} width={256} height={256} fit="cover">
          <Offset x={64} y={64} />
        </Image>
      </Canvas>
    </View>
  );
};

export default function () {
  return (
    <Tester style={{flex: 1}}>
      <ScrollView>
        <TestCase itShould="case1: This offset filter is identical to its SVG counterpart. It allows offsetting the filtered image.">
          <OffsetDemo />
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
