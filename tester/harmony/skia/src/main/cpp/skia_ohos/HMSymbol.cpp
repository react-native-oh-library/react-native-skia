
/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "HMSymbol.h"

void HMSymbol::PathOutlineDecompose(const SkPath& path, std::vector<SkPath>& paths)
{
    SkPath::RawIter iter = SkPath::RawIter(path);
    SkPoint pts[4]; // the 4 is number of points
    SkPath::Verb verb;
    SkPath path_stemp;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                if (!path_stemp.isEmpty()) {
                    paths.push_back(path_stemp);
                }
                path_stemp.reset();
                path_stemp.moveTo(pts[0]); // the 0 is first point
                break;
            case SkPath::kLine_Verb:
                path_stemp.lineTo(pts[1]); // the 1 is second point
                break;
            case SkPath::kQuad_Verb:
                path_stemp.quadTo(pts[1], pts[2]); // the 1 and 2 is second and third point
                break;
            case SkPath::kCubic_Verb:
                path_stemp.cubicTo(pts[1], pts[2], pts[3]); // the 1, 2 and 3 if the second, third and fourth point
                break;
            case SkPath::kConic_Verb:
                path_stemp.conicTo(pts[1], pts[2], iter.conicWeight()); // the 1 and 2 is second and third point
                break;
            case SkPath::kClose_Verb:
                path_stemp.close();
                break;
            case SkPath::kDone_Verb:
                if (!path_stemp.isEmpty()) {
                    paths.push_back(path_stemp);
                }
                path_stemp.reset();
                SkUNREACHABLE;
            default:
                break;
        }
    }
    if (!path_stemp.isEmpty()) {
        paths.push_back(path_stemp);
    }
}

void HMSymbol::MultilayerPath(const std::vector<std::vector<size_t>>& multMap,
    const std::vector<SkPath>& paths, std::vector<SkPath>& multPaths)
{
    if (multMap.empty()) {
        SkPath path;
        for (size_t i = 0; i < paths.size(); i++) {
            path.addPath(paths[i]);
        }
        multPaths.push_back(path);
        return;
    }
    for (size_t i = 0; i < multMap.size(); i++) {
        SkPath path;
        for (size_t j = 0; j < multMap[i].size(); j++) {
            if (multMap[i][j] < paths.size()) {
                path.addPath(paths[multMap[i][j]]);
            }
        }
        multPaths.push_back(path);
    }
}