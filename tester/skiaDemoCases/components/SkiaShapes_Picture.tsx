import React, {Component, useMemo} from 'react';
import {Text, View} from 'react-native';

import {
  Canvas,
  vec,
  Patch,
  Picture,
  Skia,
  createPicture,
  BlendMode,
  Group,
  Paint,
  Blur,
} from '@shopify/react-native-skia';

import {ScrollView} from 'react-native';
import {Tester, TestCase} from '@rnoh/testerino';

export const PictureDemo = () => {
  // Create a picture
  const picture = useMemo(
    () =>
      createPicture(canvas => {
        const size = 256;
        const r = 0.33 * size;
        const paint = Skia.Paint();
        paint.setBlendMode(BlendMode.Multiply);

        paint.setColor(Skia.Color('cyan'));
        canvas.drawCircle(r, r, r, paint);

        paint.setColor(Skia.Color('magenta'));
        canvas.drawCircle(size - r, r, r, paint);

        paint.setColor(Skia.Color('yellow'));
        canvas.drawCircle(size / 2, size - r, r, paint);
      }),
    [],
  );
  return (
    <View style={{padding: 10, alignItems: 'center'}}>
      <Canvas style={{flex: 1, width: 256, height: 256}}>
        <Picture picture={picture} />
      </Canvas>
    </View>
  );
};

export const ApplyingEffectsPictureDemo = () => {
  const picture = createPicture(canvas => {
    const size = 256;
    const r = 0.33 * size;
    const paint = Skia.Paint();
    paint.setBlendMode(BlendMode.Multiply);

    paint.setColor(Skia.Color('cyan'));
    canvas.drawCircle(r, r, r, paint);

    paint.setColor(Skia.Color('magenta'));
    canvas.drawCircle(size - r, r, r, paint);

    paint.setColor(Skia.Color('yellow'));
    canvas.drawCircle(size / 2, size - r, r, paint);
  });
  return (
    <View style={{padding: 10, alignItems: 'center'}}>
      <Group
        layer={
          <Paint>
            <Blur blur={10} />
          </Paint>
        }>
        <Picture picture={picture} />
      </Group>
    </View>
  );
};

export default function () {
  return (
    <Tester style={{flex: 1}}>
      <ScrollView>
        <TestCase itShould="case1: Picture">
          <PictureDemo />
        </TestCase>

        {/* <TestCase itShould="case2: Applying Effects Picture">
          <ApplyingEffectsPictureDemo />
        </TestCase> */}
      </ScrollView>
    </Tester>
  );
}
