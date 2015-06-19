// socket++ library. sig.h
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
// 
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.

#ifndef SIG_H
#define SIG_H

#include "socketerr.h"

#include <socketpp-config.h>

#include <list>
#include <map>
#include <algorithm>
#include <csignal>
#include <sys/types.h>

namespace socketpp
{

// all signal handlers must be derived from
// class sig::handler. class signal will
// maintain a list of pointers to sig::handler
// objects for a signal. If a signal occurs,
// all sig::handler's associated with the
// signal are invoked.

// sig::handler object will insert itself into
// the signal handler list for a signo. Its
// dtor will delete the signal handler object
// from the signal handler list for a signo.
// Thus if a user wants to add a signal handler,
// all that needs to be done is to simply
// instantiate the signal handler object,
// and if the user wants to remove the signal
// handler, all that needs to done is to
// delete the object and its dtor will remove 
// itself from the signal handler list.

// Note: you cannot mix sig with other means
//       of setting signal handlers.

class sig {
public:

  class handler 
  {
    int signo;
  public:
    handler (int signo); 
    virtual ~handler ();
    virtual void operator () (int s) = 0;
  };

  typedef handler* phnd;
  typedef std::list<phnd> phndlist;
  typedef std::map<int, phndlist, std::less<int> > sigmap;
private:
  sigmap smap;

  sig () {}
  ~sig () {}
	
public:

  /** add signal handler h for signal signo
  * return true on success. false otherwise.
  */
  bool set (int signo, handler* h);

  /**remove signal handler h for signal signo
  * return true on success. false otherwise.
  * Note: the user needs to delete h.
  */
  bool unset (int signo, handler* h);

  /// remove all signal handers and reset the handler to default for signal signo
  void unbind (int signo);

  //following functions are not available on platforms without sigaction
  #ifdef HAVE_SIGACTION
  /** mask signal signo. Prevent signo from being seen
  * by our process. Note: not all signals can be
  * blocked.
  */
  void mask (int signo) const;

  /** block signal signo_b when inside a signo_a handler
  * Note: the process will see signo_b once signo_a handler
  * is finished
  */
  void mask (int signo_a, int signo_b) const;
  
  /** unmask signal signo. Enable signo to be seen by
  * our process.
  */
  void unmask (int signo) const;

  /// unblock signal signo_b when inside a signo_a handler
  void unmask (int signo_a, int signo_b) const;
  
  /** make some system calls to terminate after they are
  * interrupted (set == false). Otherwise resume system
  * call (set == true). Not available on all systems.
  */
  void sysresume (int signo, bool set) const;
  
  /// is signal signo pending?
  bool ispending (int signo) const;

  /// set of signals pending
  sigset_t pending () const;
  
  #endif

  /// process a software signal signo
  void kill (int signo);

  /// return the only instance of sig
  static sig& instance();  
};

}

#endif // SIG_H
