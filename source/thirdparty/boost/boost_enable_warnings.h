// Re-enable all the warnings disabled by boost_disable_warnings.h.

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning( pop )
#endif
