import {
  RNPackage, TurboModulesFactory } from '@rnoh/react-native-openharmony/ts';
import type { TurboModule, TurboModuleContext } from '@rnoh/react-native-openharmony/ts';
import { RNSkiaModule } from './RNSkiaModule';


class RNSkiaTurboModulesFactory extends TurboModulesFactory {
  createTurboModule(name: string): TurboModule | null {
    if (name === 'RNSkiaModule') {
      return new RNSkiaModule(this.ctx);
    }
    return null;
  }

  hasTurboModule(name: string): boolean {
    return name === "RNSkiaModule"
  }
}

export class RNSkiaPackage extends RNPackage {
  createTurboModulesFactory(ctx: TurboModuleContext): TurboModulesFactory {
    return new RNSkiaTurboModulesFactory(ctx);
  }
}