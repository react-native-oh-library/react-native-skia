/**
 * MIT License
 *
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
import XComponentContext from '../XComponentContext';
import { display } from '@kit.ArkUI';
import Logger from '../Logger';

const TAG: string = 'SkiaDomView'
const X_COMPONENT_ID = 'SkiaDomView';

@Component
export struct SkiaDomView {
  private xComponentController: XComponentController = new XComponentController();
  private xComponentContext: XComponentContext | undefined = undefined;
  viewWidth: number = 0;
  viewHeight: number = 0;
  nativeID: number = 0;
  xComponentId: string = '';
  mode: string = 'default';
  debug: boolean = true;

  aboutToAppear(): void {
    this.xComponentId = X_COMPONENT_ID + '_' + this.nativeID;
    display.getAllDisplays((error, display) => {
      Logger.debug(TAG, 'aboutToAppear getAllDisplays' + JSON.stringify(display));
    });
  }

  aboutToDisappear(): void {
    Logger.debug(TAG, 'aboutToDisappear nativeID: ' + this.nativeID);
    this.xComponentContext?.unregisterView(X_COMPONENT_ID + "_" + this.nativeID, this.nativeID);
  }

  build() {
    XComponent({
      id: X_COMPONENT_ID + "_" + this.nativeID,
      type: XComponentType.SURFACE,
      libraryname: "rnoh_skia",
      controller: this.xComponentController
    })
      .onLoad((xComponentContext) => {
        this.xComponentContext = xComponentContext as XComponentContext;
        Logger.debug(TAG, 'XComponent onLoad nativeID=' + this.nativeID);
        this.xComponentContext?.registerView(X_COMPONENT_ID + "_" + this.nativeID, this.nativeID);
        this.xComponentContext?.setModeAndDebug(this.xComponentId, this.mode, this.debug);
      })
      .onSizeChange((oldValue: SizeOptions, newValue: SizeOptions) => {
        Logger.debug(TAG,
          'XComponent onSizeChange oldValue=' + JSON.stringify(oldValue) + ", newValue=" + JSON.stringify(newValue));
        let width = newValue.width?.valueOf() as number;
        let height = newValue.height?.valueOf() as number;
        this.xComponentContext?.onSurfaceSizeChanged(X_COMPONENT_ID + "_" + this.nativeID, this.nativeID, width, height);
      })
      .backgroundColor(Color.Transparent)
      .width(this.viewWidth)
      .height(this.viewHeight);
  }
}