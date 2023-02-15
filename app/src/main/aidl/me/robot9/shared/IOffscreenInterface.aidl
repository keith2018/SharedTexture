// OffscreenInterface.aidl
package me.robot9.shared;

// Declare any non-default types here with import statements

interface IOffscreenInterface {

    void init(in HardwareBuffer buff, int width, int height);

    ParcelFileDescriptor drawFrame();

    void destroy();
}
