/* eslint-disable prettier/prettier */
import { NativeModules, Platform } from 'react-native';
import type { Functions } from './functions/functions.ts'

const LINKING_ERROR =
  `The package 'react-native-sss' doesn't seem to be linked. Make sure: \n\n` +
  Platform.select({ ios: "- You have run 'pod install'\n", default: '' }) +
  '- You rebuilt the app after installing the package\n' +
  '- You are not using Expo Go\n';

// @ts-expect-error
const isTurboModuleEnabled = global.__turboModuleProxy != null;

const SssModule = isTurboModuleEnabled
  ? require('./NativeSss').default
  : NativeModules.Sss;

const Sss = SssModule
  ? SssModule
  : new Proxy( {} ,
    {
      get() {
        throw new Error(LINKING_ERROR);
      },
    }
  );

if (!isTurboModuleEnabled) {
  const result = Sss.install?.() as boolean;

  if (result !== true) {
    console.error('Failed to install Sss bindings!');
  }
}

declare global {
  var __loadSss: () => OpenCVModel;
}

export const sss = global.__loadSss();

export type OpenCVModel = Functions;

