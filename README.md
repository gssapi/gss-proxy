[![Changelog](https://img.shields.io/github/v/release/gssapi/gssproxy?label=changelog)](https://github.com/gssapi/gssproxy/releases)
[![Build Status](https://github.com/gssapi/gssproxy/actions/workflows/ci.yaml/badge.svg)](https://github.com/gssapi/gssproxy/actions/workflows/ci.yaml)

This is the gss-proxy project.

Documentation lives in the [docs folder of this
repository](https://github.com/gssapi/gssproxy/tree/master/docs).

The goal is to have a GSS-API proxy, with standardizable protocol and a
(somewhat portable) reference client and server implementation.  There
are several motivations for this some of which are:

 - Kernel-mode GSS-API applications (CIFS, NFS, AFS, ...) need to be
   able to leave all complexity of GSS\_Init/Accept\_sec\_context() out of
   the kernel by upcalling to a daemon that does all the dirty work.

 - Isolation and privilege separation for user-mode applications.  For
   example: letting HTTP servers use but not see the keytabe entries for
   HTTP/* principals for accepting security contexts.

 - Possibly an ssh-agent-like SSH agent for GSS credentials -- a
   gss-agent.

gss-proxy uses libverto for dealing with event loops. Note that you need to
have at least one libverto event library installed (e.g. libverto-tevent).

We have a
[mailing list](https://lists.fedorahosted.org/archives/list/gss-proxy@lists.fedorahosted.org/)
and an IRC channel (#gssapi on [libera.chat](https://libera.chat/)).
