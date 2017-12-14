// Re-enable all the warnings disabled by blaze_disable_warnings.h.

#if defined(__clang__)
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#if _MSC_VER <= 1900
#pragma warning( pop )
#endif
#endif
