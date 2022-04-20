import type {
  AnimatedProps,
  PathDef,
  PathProps,
  SkPath,
} from "@shopify/react-native-skia";
import {
  processPath,
  Easing,
  Canvas,
  Fill,
  Path,
  runTiming,
  useDerivedValue,
  useValue,
} from "@shopify/react-native-skia";
import React, { useCallback, useEffect, useRef, useState } from "react";
import { StyleSheet, Text, View } from "react-native";

import { createGraphPath } from "./createGraphPath";
import type { GraphProps } from "./types";

export const Interpolation: React.FC<GraphProps> = ({ height, width }) => {
  const [currentPath, setCurrentPath] = useState<SkPath>(() =>
    createGraphPath(width, height, 60)
  );

  const onPress = useCallback(() => {
    setCurrentPath(createGraphPath(width, height, 60));
  }, [height, width]);

  return (
    <View style={{ height, marginBottom: 10 }} onTouchEnd={onPress}>
      <Canvas style={styles.graph}>
        <Fill color="black" />
        <PathTransition
          path={currentPath}
          strokeWidth={4}
          style="stroke"
          strokeJoin="round"
          strokeCap="round"
          color="#cccc66"
        />
      </Canvas>
      <Text>Touch graph to interpolate</Text>
    </View>
  );
};

const PathTransition = ({
  path,
  ...props
}: AnimatedProps<PathProps> & { path: PathDef }) => {
  // Save current and next paths (initially the same)
  const currentPathRef = useRef(processPath(path));
  const nextPathRef = useRef(processPath(path));

  // Progress value drives the animation
  const progress = useValue(0);

  // The animated path is derived from the current and next paths based
  // on the value of the progress.
  const animatedPath = useDerivedValue(
    () =>
      nextPathRef.current.interpolate(currentPathRef.current, progress.current),
    [progress, path]
  );

  useEffect(() => {
    if (currentPathRef.current !== path) {
      // Process path - can be an SVG string
      const processedPath = processPath(path);

      // Ensure paths have the same length
      if (
        currentPathRef.current.countPoints() !== processedPath.countPoints()
      ) {
        console.warn(
          "Paths must have the same length. Skipping interpolation."
        );
        return;
      }
      // Stop any ongoing animations
      progress.animation = undefined;
      // Set current path to the current interpolated path to make
      // sure we can interrupt animations
      currentPathRef.current = nextPathRef.current.interpolate(
        currentPathRef.current,
        progress.current
      );
      // Set the next path to be the value in the updated path property
      nextPathRef.current = processedPath;
      // reset progress - this will cause the derived value to be updated and
      // the path to be repainted through its parent canvas.
      progress.current = 0;
      // Run animation
      runTiming(
        progress,
        1,
        { duration: 750, easing: Easing.inOut(Easing.cubic) },
        () => {
          // Update main path to the next path
          currentPathRef.current = processedPath;
          // Reset progress
          progress.current = 0;
        }
      );
    }
  }, [animatedPath, path, progress]);

  return <Path {...props} path={animatedPath} />;
};

PathTransition.defaultProps = {
  start: 0,
  end: 1,
};

const styles = StyleSheet.create({
  graph: {
    flex: 1,
  },
});
