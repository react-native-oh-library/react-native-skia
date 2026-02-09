/**
 * MIT License
 *
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

import { BusinessError } from '@kit.BasicServicesKit';
import { TurboModule, TurboModuleContext } from '@rnoh/react-native-openharmony/ts';
import fs from '@ohos.file.fs';
import { componentSnapshot } from '@kit.ArkUI';
import { image } from '@kit.ImageKit';
import testNapi from 'librnoh_skia.so';


type Options = {
  width?: number,
  height?: number,
  stride: number,
  pixelFormat: number,
  alphaType: number,
};

export class RNSkiaModule extends TurboModule {
  public static readonly NAME = 'RNSkiaModule';

  options: Options | null = null;

  public TagGetView(tag: number): void {
    try {
      let id = JSON.parse(JSON.stringify(this.ctx.rnInstance.getNativeNodeIdByTag(tag)));
      if (typeof id === "object") {
        id = id.ok
      }
      let pixelmap = componentSnapshot.getSync(id);
      let originImageInfo = pixelmap.getImageInfoSync();
      let dstX = originImageInfo.size.width;
      let dstY = originImageInfo.size.height;
      this.options = {
        width: dstX,
        height: dstY,
        stride: originImageInfo.stride,
        pixelFormat: originImageInfo.pixelFormat,
        alphaType: originImageInfo.alphaType,
      };
      testNapi.TagGetView_s(dstX,dstY,originImageInfo.stride,originImageInfo.pixelFormat,originImageInfo.alphaType,pixelmap);
    } catch (error) {
      console.error("Error occurred while calling getSync:", error);
      return;
    }
  }
}

