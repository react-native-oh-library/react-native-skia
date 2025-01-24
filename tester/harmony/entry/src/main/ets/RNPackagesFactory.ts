import type { RNPackageContext, RNPackage } from '@rnoh/react-native-openharmony/ts';
import { ReanimatedPackage } from '@react-native-oh-tpl/react-native-reanimated/ts';
import { GestureHandlerPackage } from '@react-native-oh-tpl/react-native-gesture-handler/ts';
import { RNSkiaPackage } from '@react-native-oh-tpl/react-native-skia/ts';

export function createRNPackages(ctx: RNPackageContext): RNPackage[] {
  return [
    new GestureHandlerPackage(ctx),
    new ReanimatedPackage(ctx),
    new RNSkiaPackage(ctx),
  ];
}
