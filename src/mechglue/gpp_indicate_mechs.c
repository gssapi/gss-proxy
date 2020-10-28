/* Copyright (C) 2012 the GSS-PROXY contributors, see COPYING for license */

#include "gss_plugin.h"

/* This will never be called, added only for completeness */
OM_uint32 gssi_indicate_mechs(OM_uint32 *minor_status, gss_OID_set *mech_set)
{
    GSSI_TRACE();
    *minor_status = 0;
    return GSS_S_FAILURE;
}

OM_uint32 gssi_inquire_names_for_mech(OM_uint32 *minor_status,
                                      gss_OID mech_type,
                                      gss_OID_set *mech_names)
{
    enum gpp_behavior behavior;
    OM_uint32 tmaj, tmin;
    OM_uint32 maj, min;

    GSSI_TRACE();

    behavior = gpp_get_behavior();
    tmaj = GSS_S_COMPLETE;
    tmin = 0;

    /* See if we should try local first */
    if (behavior == GPP_LOCAL_ONLY || behavior == GPP_LOCAL_FIRST) {

        maj = gss_inquire_names_for_mech(&min,
                                         gpp_special_mech(mech_type),
                                         mech_names);
        if (maj == GSS_S_COMPLETE || behavior == GPP_LOCAL_ONLY) {
            goto done;
        }

        /* not successful, save actual local error if remote fallback fails */
        tmaj = maj;
        tmin = min;
    }

    /* Then try with remote */
    if (behavior != GPP_LOCAL_ONLY) {

        maj = gpm_inquire_names_for_mech(&min, mech_type, mech_names);
        if (maj == GSS_S_COMPLETE || behavior == GPP_REMOTE_ONLY) {
            goto done;
        }

        /* So remote failed, but we can fallback to local, try that */
        maj = gss_inquire_names_for_mech(&min,
                                         gpp_special_mech(mech_type),
                                         mech_names);
    }

done:
    if (maj != GSS_S_COMPLETE && tmaj != GSS_S_COMPLETE) {
        maj = tmaj;
        min = tmin;
    }
    *minor_status = gpp_map_error(min);
    return maj;
}

OM_uint32 gssi_inquire_attrs_for_mech(OM_uint32 *minor_status,
                                      gss_OID mech,
                                      gss_OID_set *mech_attrs,
                                      gss_OID_set *known_mech_attrs)
{
    enum gpp_behavior behavior;
    OM_uint32 tmaj, tmin;
    OM_uint32 maj, min;

    GSSI_TRACE();

    behavior = gpp_get_behavior();
    tmaj = GSS_S_COMPLETE;
    tmin = 0;

    /* See if we should try local first */
    if (behavior == GPP_LOCAL_ONLY || behavior == GPP_LOCAL_FIRST) {

        maj = gss_inquire_attrs_for_mech(&min, gpp_special_mech(mech),
                                         mech_attrs, known_mech_attrs);
        if (maj == GSS_S_COMPLETE || behavior == GPP_LOCAL_ONLY) {
            goto done;
        }

        /* not successful, save actual local error if remote fallback fails */
        tmaj = maj;
        tmin = min;
    }

    /* Then try with remote */
    if (behavior != GPP_LOCAL_ONLY) {

        maj = gpm_inquire_attrs_for_mech(&min, mech,
                                         mech_attrs, known_mech_attrs);
        if (maj == GSS_S_COMPLETE || behavior == GPP_REMOTE_ONLY) {
            goto done;
        }

        /* So remote failed, but we can fallback to local, try that */
        maj = gss_inquire_attrs_for_mech(&min, gpp_special_mech(mech),
                                         mech_attrs, known_mech_attrs);
    }

done:
    if (maj != GSS_S_COMPLETE && tmaj != GSS_S_COMPLETE) {
        maj = tmaj;
        min = tmin;
    }
    *minor_status = gpp_map_error(min);
    return maj;
}

OM_uint32 gssi_inquire_saslname_for_mech(OM_uint32 *minor_status,
                                         const gss_OID desired_mech,
                                         gss_buffer_t sasl_mech_name,
                                         gss_buffer_t mech_name,
                                         gss_buffer_t mech_description)
{
    enum gpp_behavior behavior;
    OM_uint32 tmaj, tmin;
    OM_uint32 maj, min;

    GSSI_TRACE();

    behavior = gpp_get_behavior();
    tmaj = GSS_S_COMPLETE;
    tmin = 0;

    /* See if we should try local first */
    if (behavior == GPP_LOCAL_ONLY || behavior == GPP_LOCAL_FIRST) {

        maj = gss_inquire_saslname_for_mech(&min,
                                            gpp_special_mech(desired_mech),
                                            sasl_mech_name, mech_name,
                                            mech_description);
        if (maj == GSS_S_COMPLETE || behavior == GPP_LOCAL_ONLY) {
            goto done;
        }

        /* not successful, save actual local error if remote fallback fails */
        tmaj = maj;
        tmin = min;
    }

    /* Then try with remote */
    if (behavior != GPP_LOCAL_ONLY) {

        maj = gpm_inquire_saslname_for_mech(&min, desired_mech, sasl_mech_name,
                                            mech_name, mech_description);
        if (maj == GSS_S_COMPLETE || behavior == GPP_REMOTE_ONLY) {
            goto done;
        }

        /* So remote failed, but we can fallback to local, try that */
        maj = gss_inquire_saslname_for_mech(&min,
                                            gpp_special_mech(desired_mech),
                                            sasl_mech_name, mech_name,
                                            mech_description);
    }

done:
    if (maj != GSS_S_COMPLETE && tmaj != GSS_S_COMPLETE) {
        maj = tmaj;
        min = tmin;
    }
    *minor_status = gpp_map_error(min);
    return maj;
}

OM_uint32 gssi_inquire_mech_for_saslname(OM_uint32 *minor_status,
                                         const gss_buffer_t sasl_mech_name,
                                         gss_OID *mech_type)
{
    GSSI_TRACE();
    /* NOTE: in general it doesn't make a lot of sense to provide this
     * function unless we make it possible to export an entire mechanism
     * via interposer functions that is not available in the main library.
     * That is because any saslname we can interpose is actually going to
     * be properly resolved by the main mechanism, and when later the
     * returned mechanism type is used in some call we will properly
     * interpose as configured. */
    return GSS_S_UNAVAILABLE;
}
