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

const TAG: string = 'RNOH in ctrl'
const X_COMPONENT_ID = 'SkiaDomView';

@Component
export struct SkiaDomView {
  private xComponentController: XComponentController = new XComponentController();
  private surfaceID: number = 0;

  private xComponentContext: XComponentContext | undefined = undefined;

  viewWidth: number = 0;
  viewHeight: number = 0;
  nativeID: number = 0;


  aboutToAppear(): void {

  }


  build() {
    Column() {
      XComponent({
        id: X_COMPONENT_ID,
        type: XComponentType.TEXTURE,
        libraryname: "rnoh_skia",
        controller: this.xComponentController
      }).onLoad((xComponentContext) => {
        this.xComponentContext = xComponentContext as XComponentContext;
        this.surfaceID = Number(this.xComponentController.getXComponentSurfaceId());
        console.log('liwang----------->XComponent onLoad nativeID=' + this.nativeID + ", surfaceID=" + this.surfaceID);
        this.xComponentContext.registerView(X_COMPONENT_ID, this.nativeID);
      })
      .width(this.viewWidth)
      .height(this.viewHeight)
    }
    .width(this.viewWidth)
    .height(this.viewHeight)
    .alignItems(HorizontalAlign.Center)
    .justifyContent(FlexAlign.Center);
  }
}