// socket++ library. sig.C 
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.

#include <signal.h>

#include <base/sig.h>

namespace socketpp
{

//explicit template instantiation.
typedef sig::phnd phnd;
typedef sig::phndlist phndlist;


typedef void (*sighnd_type) (int);

extern "C" {
  static void sighandler (int signo) {
    sig::instance().kill (signo);
  }
}

sig::handler::handler(int s)
  : signo(s)
{
  sig::instance().set (signo, this);
}

sig::handler::~handler ()
{
  sig::instance().unset (signo, this);
}

sig& sig::instance()
{
	//will be created on first use
	static sig sigInstance;
	
	return sigInstance;
}

bool sig::set(int signo, sig::handler* handler)
{
  if (handler == 0) return false;

  phndlist& v = smap[signo];
  
  if (v.empty ()) 
  {
	
	//fancy code that uses the latest POSIX-recommended function, sigaction
	#ifndef HAVE_SIGACTION
    struct sigaction sa;
	
	//read the current signal information into sa
    if (sigaction (signo, 0, &sa) == -1)
	{
		throw cpr::exception(errno, "sig::set");
	}
	
    if (sa.sa_handler != sighnd_type (&sighandler)) 
	{
      // setting for the first time
      sa.sa_handler = (void(*)(int)) sighnd_type (&sighandler);
      if (sigemptyset (&sa.sa_mask) == -1) throw cpr::exception(errno, "sig::set");
      sa.sa_flags = 0;
      if (sigaction(signo, &sa, 0) == -1) throw cpr::exception(errno, "sig::set");
    }
	#else //HAVE_SIGACTION
	//older code compatible with Windows
	signal(signo, &sighandler);
	#endif
	
    v.push_back (handler);
    return true;
  }

  phndlist::iterator j = find (v.begin(), v.end (), handler);
  if (j == v.end ()) {
    v.push_back (handler);
    return true;
  }
  return false;
}

bool sig::unset (int signo, sig::handler* handler)
{
  if (handler == 0) return false;
  
  phndlist& v = smap [signo];

  phndlist::iterator j = find (v.begin(), v.end (), handler);
  if (j != v.end ()) {
    v.erase (j);
    return true;
  }

  return false;
}

void sig::unbind(int signo)
{
  phndlist& v = smap [signo];
  v.erase (v.begin (), v.end ());
  
  #ifndef HAVE_SIGACTION
  struct sigaction sa;
  if (sigaction (signo, 0, &sa) == -1) throw cpr::exception(errno, "sig::unbind");
  
  //check if handler is set to our function
  if (sa.sa_handler == sighnd_type (&sighandler))
  {
	//reset handlerr to default
    sa.sa_handler = (void(*)(int)) sighnd_type (SIG_DFL);
    if (sigemptyset (&sa.sa_mask) == -1) throw cpr::exception(errno, "sig::unbind");
    sa.sa_flags = 0;
    if (sigaction (signo, &sa, 0) == -1) throw cpr::exception(errno, "sig::unbind");
  }
  #else
  signal(signo, SIG_DFL);
  #endif
}

#ifndef HAVE_SIGACTION
void sig::mask (int signo) const
{
  sigset_t s;
  if (sigemptyset (&s) == -1) throw cpr::exception(errno, "sig::mask");
  if (sigaddset (&s, signo) == -1) throw cpr::exception(errno, "sig::mask");

  if (sigprocmask (SIG_BLOCK, &s, 0) == -1) throw cpr::exception(errno, "sig::mask");
}

void sig::unmask (int signo) const
{
  sigset_t s;
  if (sigemptyset (&s) == -1) throw cpr::exception(errno, "sig::unmask");
  if (sigaddset (&s, signo) == -1) throw cpr::exception(errno, "sig::unmask");

  if (sigprocmask (SIG_UNBLOCK, &s, 0) == -1) throw cpr::exception(errno, "sig::unmask");
}

void sig::mask (int siga, int sigb) const
{
  struct sigaction sa;
  if (sigaction (siga, 0, &sa) == -1) throw cpr::exception(errno, "sig::mask");
  if (sa.sa_handler != sighnd_type (&sighandler)) {
    sa.sa_handler = (void(*)(int)) sighnd_type (&sighandler);
    if (sigemptyset (&sa.sa_mask) == -1) throw cpr::exception(errno, "sig::mask");
    sa.sa_flags = 0;
  }
  if (sigaddset (&sa.sa_mask, sigb) == -1) throw cpr::exception(errno, "sig::mask");
  if (sigaction (siga, &sa, 0) == -1) throw cpr::exception(errno, "sig::mask");
}

void sig::unmask (int siga, int sigb) const
{
  struct sigaction sa;
  if (sigaction (siga, 0, &sa) == -1) throw cpr::exception(errno, "sig::unmask");
  if (sa.sa_handler != sighnd_type (&sighandler)) {
    sa.sa_handler = (void(*)(int)) sighnd_type (&sighandler);
    if (sigemptyset (&sa.sa_mask) == -1) throw cpr::exception(errno, "sig::unmask");
    sa.sa_flags = 0;
  } else {
    if (sigdelset (&sa.sa_mask, sigb) == -1) throw cpr::exception(errno, "sig::unmask");
  }
  if (sigaction (siga, &sa, 0) == -1) throw cpr::exception(errno, "sig::unmask");
}



void sig::sysresume (int signo, bool set) const
{
  struct sigaction sa;
  if (sigaction (signo, 0, &sa) == -1) throw cpr::exception(errno, "sig::sysresume");
  if (sa.sa_handler != sighnd_type (&sighandler)) {
    sa.sa_handler = (void(*)(int)) sighnd_type (&sighandler);
    if (sigemptyset (&sa.sa_mask) == -1) throw cpr::exception(errno, "sig::sysresume");
    sa.sa_flags = 0;
  }

#ifdef SA_INTERRUPT
// Early SunOS versions may have SA_INTERRUPT. I can't confirm.
  if (set == false)
    sa.sa_flags |= SA_INTERRUPT;
  else
    sa.sa_flags &= ~SA_INTERRUPT;
  if (sigaction (signo, &sa, 0) == -1) throw cpr::exception(errno, "sig::sysresume");
#endif
}

sigset_t sig::pending () const
{
  sigset_t s;
  if (sigemptyset (&s) == -1) throw cpr::exception(errno, "sig::pending");
  if (sigpending (&s) == -1) throw cpr::exception(errno, "sig::pending");
  return s;
}

bool sig::ispending (int signo) const
{
  sigset_t s = pending ();
  switch (sigismember (&s, signo)) {
  case 0: return false;
  case 1: return true;
  }
  throw cpr::exception(errno, "sig::ispending");
}

#endif

struct procsig {
  int signo;
  procsig (int s): signo (s) {}
  void operator () (phnd& ph) { (*ph) (signo); }
};

void sig::kill (int signo)
{
  phndlist& v = smap [signo];

  // struct procsig used to be here // LN
  
  for(phnd handler : v)
  {
  	(*handler)(signo);
  }
}

}
