import type {RNPackageContext, RNPackage} from '@rnoh/react-native-openharmony/ts';
import {RNSkiaPackage} from '@react-native-oh-tpl/react-native-skia/ts';
import { ReanimatedPackage } from '@react-native-oh-tpl/react-native-reanimated/ts';

export function createRNPackages(ctx: RNPackageContext): RNPackage[] {
  return [new RNSkiaPackage(ctx), new ReanimatedPackage(ctx)];
}
