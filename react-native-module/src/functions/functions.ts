export type Functions = {
  /**
   * Tests basic OpenCV by doing a canny detection.
   * @param base64 - the base64 string
   */
  test(base64: string): {
    base64: string;
  };
  /**
   * Process an image.
   * @param base64 - the base64 string
   */
  processImage(base64: string): {
    output: string;
  };
  /**
   * Tests basic OpenCV and worklets by doing a canny detection on a frame.
   * @param rows - the number of rows in the Mat
   * @param cols - the number of columns in the Mat
   * @param channels - the number of channels in the Mat
   * @param input - the byte array to convert
   */
  testFrame(
    rows: number,
    cols: number,
    channels: number,
    input: Uint8Array
  ): {
    base64: string;
  };
  /**
   * Process a frame.
   * @param rows - the number of rows in the Mat
   * @param cols - the number of columns in the Mat
   * @param channels - the number of channels in the Mat
   * @param input - the byte array to convert
   */
  processFrame(
    rows: number,
    cols: number,
    channels: number,
    input: Uint8Array
  ): {
    output: string;
  };
};
