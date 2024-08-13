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

import {
  Descriptor,
  RNOHContext,
  ViewBaseProps,
  RNViewBase,
  ViewRawProps,
} from '@rnoh/react-native-openharmony'
import { SkiaDomView } from './view/SkiaDomView';
import Logger from './Logger';

export interface SkiaDomViewState {}
export interface SkiaDomViewProps extends ViewRawProps {
  mode: string
  debug: boolean
}

export const SKIA_DOM_VIEW_TYPE: string = "SkiaDomView"

export type SkiaDomViewDescriptor = Descriptor<"SkiaDomView", ViewBaseProps, SkiaDomViewState, SkiaDomViewProps>

const TAG: string = 'RNOH in SkiaDomView'

@Component
export struct RNCSkiaDomView {
  ctx!: RNOHContext;
  tag: number = -1;
  @State descriptor: SkiaDomViewDescriptor = {} as SkiaDomViewDescriptor;
  private callbackList:Array<() => void> = []

  @Provide('mode') mode: string = "";
  @Provide('debug') debug: boolean = false;
  @State nativeID: number = 0;
  @State viewWidth: number = 0;
  @State viewHeight: number = 0;


  aboutToAppear(): void {
    this.descriptor = this.ctx.descriptorRegistry.getDescriptor<SkiaDomViewDescriptor>(this.tag);
    Logger.debug(TAG, `descriptor rawProps, ${JSON.stringify(this.descriptor.rawProps)}`);
    Logger.debug(TAG, `descriptor layoutMetrics, ${JSON.stringify(this.descriptor.layoutMetrics)}`);
    this.callbackList.push(this.ctx.descriptorRegistry.subscribeToDescriptorChanges(this.tag,
      (newDescriptor) => {
        this.descriptor = (newDescriptor as SkiaDomViewDescriptor);

        /*------ 当RN侧属性有更改 -------*/
        Logger.debug(TAG, `newDescriptor rawProps, ${JSON.stringify(this.descriptor.rawProps)}`);
        this.updatePropFromDesc();

      }
    ))

    /*------ 初始化 -------*/
    this.updatePropFromDesc();
    this.registerCommandCallback();

  }

  aboutToDisappear(): void {
    this.cancellationCallback()
  }

  registerCommandCallback(): void {
    if (this.ctx){
      this.callbackList.push(this.ctx.componentCommandReceiver.registerCommandCallback(
        this.tag,
        (command, args:(string|number)[]) => {
          Logger.debug(TAG, `registerCommandCallback,  ${JSON.stringify(args)}`);
      }))
    }
  }

  /* 更新属性变更  */
  updatePropFromDesc(): void {
    this.mode = this.descriptor.rawProps.mode
    this.debug = this.descriptor.rawProps.debug
    this.nativeID = Number.parseInt(this.descriptor.rawProps.nativeID?this.descriptor.rawProps.nativeID:'0');
    this.viewWidth = this.descriptor.layoutMetrics.frame.size.width
    this.viewHeight = this.descriptor.layoutMetrics.frame.size.height
  }

  cancellationCallback() {
    this.callbackList.forEach(
      callback => {
        callback?.()
      }
    )
  }

  build() {
    RNViewBase({ ctx: this.ctx, tag: this.tag }) {
      SkiaDomView({nativeID: this.nativeID, viewWidth: this.viewWidth, viewHeight: this.viewHeight })
        .align(Alignment.Center)
        .height(this.viewHeight)
        .width(this.viewWidth)
    }
  }
}