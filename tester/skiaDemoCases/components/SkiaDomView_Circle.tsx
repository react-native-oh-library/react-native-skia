import React, {Component} from 'react';
import {View} from 'react-native';

import {Canvas, Circle, Group} from '@shopify/react-native-skia';

import {ScrollView} from 'react-native';
import {Tester, Filter, TestCase, TestSuite} from '@rnoh/testerino';

class CircleExample extends Component {
  static title = 'Circle in Group';

  render() {
    const width = 256;
    const height = 256;
    const r = width * 0.33;
    return (
      <View style={{flex: 1, padding: 30}}>
        <Canvas style={{width, height}}>
          <Group blendMode="multiply" color={'green'}>
            <Circle cx={r} cy={r} r={r} color="cyan" />
            <Circle cx={width - r} cy={r} r={r} color="magenta" />
            <Circle cx={width / 2} cy={width - r} r={r} color="yellow" />
          </Group>
        </Canvas>
      </View>
    );
  }
}

export default function () {
  return (
    <Tester style={{flex: 1}}>
      <ScrollView>
        <TestCase itShould="case1: Circle in Group">
          <CircleExample />
        </TestCase>
      </ScrollView>
    </Tester>
  );
}
