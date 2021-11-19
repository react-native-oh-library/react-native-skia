import type { SkJsiInstane } from "./JsiInstance";
export enum BlurStyle {
  Normal, //!< fuzzy inside and outside
  Solid, //!< solid inside, fuzzy outside
  Outer, //!< nothing inside, fuzzy outside
  Inner, //!< fuzzy inside, nothing outside
}

export type MaskFilter = SkJsiInstane<"MaskFilter">;

/**
 * See SkMaskFilter.h for more details.
 */
export interface MaskFilterFactory {
  /**
   * Create a blur maskfilter
   * @param style
   * @param sigma - Standard deviation of the Gaussian blur to apply. Must be > 0.
   * @param respectCTM - if true the blur's sigma is modified by the CTM.
   */
  MakeBlur(style: BlurStyle, sigma: number, respectCTM: boolean): MaskFilter;
}