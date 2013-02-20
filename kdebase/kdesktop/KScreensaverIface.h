
#ifndef __KScreensaverIface_h__
#define __KScreensaverIface_h__

#include <dcopobject.h>

class KScreensaverIface : virtual public DCOPObject
{
  K_DCOP
public:

k_dcop:
  /** Lock the screen now even if the screensaver does not lock by default. */
  virtual void lock() = 0;

  /** Save the screen now.  If the user has locking enabled, the screen is
   * locked also. */
  virtual void save() = 0;

  /** Quit the screensaver if it is running */
  virtual void quit() = 0;

  /** Is the screensaver enabled? */
  virtual bool isEnabled() = 0;

  /**
   * Enable/disable the screensaver
   * returns true if the action succeeded
   */
  virtual bool enable( bool e ) = 0;

  /** Is the screen currently blanked? */
  virtual bool isBlanked() = 0;

  /** Reload the screensaver configuration. */
  virtual void configure() = 0;

  /** Only blank the screen (and possibly lock).  Do not use a custom
   * screen saver in the interest of saving battery.
   */
  virtual void setBlankOnly( bool blankOnly ) = 0;

  /***
   * @internal
  */
  virtual void saverLockReady() = 0;
};

#endif
