/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 2008 Sun Microsystems, Inc. All rights reserved.
 *
 * THE BSD LICENSE
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. 
 *
 * Neither the name of the  nor the names of its contributors may be
 * used to endorse or promote products derived from this software without 
 * specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * MODULE:      plist.c
 *
 * DESCRIPTION:
 *
 *      This module implements property lists.  A property list is an
 *      ordered array of property values.  Each property value has an
 *      handle for some data item, and may have a reference to
 *      another property list which describes the type of the data
 *      item.  Each property value has a property index which specifies
 *      its position in the property list.  A property value may also
 *      have a name.  Since the data item associated with a property
 *      value may reference another property list, it is possible to
 *      construct arbitrary linked structures of property lists.
 *
 * IMPLEMENTATION NOTES:
 */

#include "netsite.h"
#include "base/plist.h"
#include "plist_pvt.h"

int plistHashSizes[] = PLSTSIZES;

/*
 * FUNCTION:    PListAssignValue
 *
 * DESCRIPTION:
 *
 *      This function sets the value and/or type of a defined property
 *      in given property list.  If the property type is specified as
 *      NULL, it is unchanged.  However, the property value is always
 *      set to the specified value.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *      pname                   - the property name
 *      pvalue                  - the new property value
 *      ptype                   - the new property type, or NULL
 *
 * RETURNS:
 *
 *      If successful, the property index of the referenced property is
 *      returned as the function value.  Errors are indicated by a
 *      negative return code as defined in plist.h.
 */

NSAPI_PUBLIC int
PListAssignValue(PList_t plist, const char *pname,
                 const void *pvalue, PList_t ptype)
{
    PListStruct_t *pl = (PListStruct_t *)plist;
    PLValueStruct_t *pv;
    int pindex;
    int i;

    if (!plist) return ERRPLUNDEF;

    /* Got a symbol table for this property list? */
    if (pl->pl_symtab) {

        /* Yes, compute hash of specified name */
        i = PListHashName(pl->pl_symtab, pname);

        /* Search hash collision list for matching name */
        for (pv = pl->pl_symtab->pt_hash[i]; pv; pv = pv->pv_next) {

            if (!strcmp(pname, pv->pv_name)) {

                /* Name match, get property index */
                pindex = pv->pv_pi;

                /* Set the new value */
                pv->pv_value = (char *)pvalue;

                /* Set type if type is given */
                if (ptype) pv->pv_type = (PListStruct_t *)ptype;

                /* Return the property index */
                return pindex;
            }
        }
    }

    /* Error - specified property name is undefined */
    return ERRPLUNDEF;
}

/*
 * FUNCTION:    PListCreate
 *
 * DESCRIPTION:
 *
 *      This function creates a new property list and returns a handle for
 *      it.  It allows the caller to reserve a specified number of
 *      property indices at the beginning of the list, and also to limit
 *      the total number of property values that may be added to the list.
 *
 * ARGUMENTS:
 *
 *      mempool                 - handle for a memory pool to be associated
 *                                with the new property list
 *      resvprop                - number of reserved property indices
 *      maxprop                 - maximum number of properties in list
 *                                (zero or negative imposes no limit)
 *      flags                   - unused, reserved, must be zero
 *
 * RETURNS:
 *
 *      If successful, the function return value is a handle for the new
 *      property list.  Otherwise NULL is returned.
 */

NSAPI_PUBLIC PList_t
PListCreate(pool_handle_t *mempool, int resvprop, int maxprop, int flags)
{
    PListStruct_t *plist;       /* pointer to property list structure */
    int i;

    plist = (PListStruct_t *)pool_malloc(mempool, sizeof(PListStruct_t));
    if (plist) {

        /* Negative maxprop is the same as zero, i.e. no limit */
        if (maxprop < 0) maxprop = 0;

        /* If resvprop and maxprop are both specified, limit resvprop */
        if (resvprop > 0) {
            if (maxprop && (resvprop > maxprop)) resvprop = maxprop;
        }
        else resvprop = 0;

        /* Initialize property list structure */
        plist->pl_mempool = mempool;
        plist->pl_symtab = NULL;
        plist->pl_maxprop = maxprop;
        plist->pl_resvpi = resvprop;
        plist->pl_initpi = resvprop;
        plist->pl_lastpi = resvprop;

        /* Set initialize size for array of property value pointers */
        plist->pl_cursize = (resvprop) ? resvprop : PLIST_DEFSIZE;

        /* Allocate the initial array of property value pointers */
        plist->pl_ppval = (pb_entry **)pool_malloc(mempool,
                                   	           (plist->pl_cursize *
                                                    sizeof(PLValueStruct_t *)));
        if (!plist->pl_ppval) {

            /* Failed - insufficient memory */
            pool_free(mempool, (void *)plist);
            plist = NULL;
        }
        else {
            /* NULL out pointers in the reserved index range, if any */
            for (i = 0; i < plist->pl_lastpi; ++i) {
                plist->pl_ppval[i] = 0;
            }
        }
    }

    return (PList_t)plist;
}

/*
 * FUNCTION:    PListDefProp
 *
 * DESCRIPTION:
 *
 *      This function creates a new property in a specified property list.
 *      The 'pindex' argument may be used to request a particular property
 *      index for the new property.  If 'pindex' is greater than zero,
 *      the specified value is used as the new property's index, provided
 *      there is no property at that index already.  If 'pindex' is zero,
 *      then the next available property index is assigned to the new
 *      property.  A name may optionally be specified for the new property.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *      pindex                  - new property index (or zero)
 *      pname                   - new property name (or NULL)
 *
 * RETURNS:
 *
 *      If successful, the index of the new property is returned as the
 *      function value.  Errors are indicated by a negative return code
 *      as defined in plist.h.
 */

NSAPI_PUBLIC int
PListDefProp(PList_t plist, int pindex, const char *pname, const int flags)
{
    PListStruct_t *pl = (PListStruct_t *)plist;
    PLValueStruct_t *pv;

    if (!plist) return ERRPLUNDEF;

    /* Is pindex specified? */
    if (pindex > 0) {

        /* Yes, is it in the reserved range? */
        if (flags != PLFLG_IGN_RES && pindex > pl->pl_resvpi) {
            /* No, error */
            return ERRPLINVPI;
        }

        PLValueStruct_t **ppval = (PLValueStruct_t **)(pl->pl_ppval);
        if (ppval[pindex - 1]) {
            /* Error - property already exists at specified index */
            return ERRPLEXIST;
        }
    }
    else {

        /* Look for a free property index */
        pindex = PListGetFreeIndex(pl);
        if (pindex < 1) {
            /* Error - no free property index */
            return pindex;
        }
    }

    /* We have a property index.  Create a new property value */
    pv = (PLValueStruct_t *)pool_calloc(pl->pl_mempool,
                                        1, sizeof(PLValueStruct_t));
    if (!pv) {

        /* Error - insufficient memory */
        return ERRPLNOMEM;
    }

    PLValueStruct_t **ppval = (PLValueStruct_t **)(pl->pl_ppval);
    pv->pv_pbentry.param = &pv->pv_pbparam;
    pv->pv_pi = pindex;
    pv->pv_mempool = pl->pl_mempool;
    ppval[pindex - 1] = pv;

    /* Name the property if the name was specified */
    if (pname) {

        /* XXX Maybe should delete property if naming fails */
        return PListNameProp(plist, pindex, pname);
    }

    /* Return the property index of the new property */
    return pindex;
}

/*
 * FUNCTION:    PListDeleteProp
 *
 * DESCRIPTION:
 *
 *      This function deletes a property from a specified property list.
 *      The property can be specified by its property index, using a
 *      pindex value greater than zero, or by its name, by specifying
 *      pindex as zero and pname as the property name.  This does not
 *      have any effect on the data referenced by the property value,
 *      if any, nor does it have any effect on the property list that
 *      describes the property value's type, if any.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *      pindex                  - the property index, or zero
 *      pname                   - the property name, or NULL
 */

NSAPI_PUBLIC const void *
PListDeleteProp(PList_t plist, int pindex, const char *pname_in)
{
    PListStruct_t *pl = (PListStruct_t *)plist;
    PLValueStruct_t **ppval;
    PLValueStruct_t **pvp;
    PLValueStruct_t *pv = NULL;
    int i;
    const void *pvalue = NULL;
    char *pname = (char *)pname_in;

    if (!plist) return NULL;

    ppval = (PLValueStruct_t **)(pl->pl_ppval);

    /* Check for valid property index */
    if ((pindex > 0) && (pindex <= pl->pl_initpi)) {

        /* Get the pointer to the property structure */
        pv = ppval[pindex - 1];
	pname = 0;
	if (pv) {
	    pname = pv->pv_name;
	}
    }

    if (pname && pl->pl_symtab) {

        /* Compute hash of specified property name */
        i = PListHashName(pl->pl_symtab, pname);

        /* Search hash collision list for matching name */
        for (pvp = &pl->pl_symtab->pt_hash[i]; *pvp; pvp = &(*pvp)->pv_next) {

            pv = *pvp;
            if (!strcmp(pname, pv->pv_name)) {

                /* Found it.  Get its index and remove it. */
                pindex = pv->pv_pi;
                *pvp = pv->pv_next;
                pl->pl_symtab->pt_nsyms--;
                break;
            }
            pv = NULL;
        }
    }

    /* Found the indicated property by index or name? */
    if (pv) {

        /* Yes, remove it from the property list */
        ppval[pindex - 1] = NULL;

        /* Free the property name, if any */
        if (pv->pv_name) {
            pool_free(pv->pv_mempool, (void *)(pv->pv_name));
        }
        pvalue = pv->pv_value;

        /* Free the property */
        pool_free(pv->pv_mempool, (void *)pv);
    }
    return(pvalue);
}

/*
 * FUNCTION:    PListFindValue
 *
 * DESCRIPTION:
 *
 *      This function retrieves the value and type of a property with a
 *      specified property name.  If the pvalue argument is non-NULL,
 *      it specifies a location in which to return the property value.
 *      Similarly, if ptype is non-NULL, it specifies where the property
 *      list describing the property type is to be returned.  If a
 *      property has no value, the value returned for pvalue is NULL.
 *      If a property has no type, the value returned for ptype is NULL.
 *      A property can have a value, a type, both, or neither.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *      pname                   - pointer to property name string
 *      pvalue                  - property value return pointer
 *      ptype                   - property type return pointer
 *
 * RETURNS:
 *
 *      If successful, the index of the referenced property is returned
 *      as the function value.  Errors are indicated by a negative
 *      return code as defined in plist.h.
 */

NSAPI_PUBLIC int
PListFindValue(PList_t plist, const char *pname, void **pvalue, PList_t *ptype)
{
    PListStruct_t *pl = (PListStruct_t *)plist;
    PLValueStruct_t *pv;
    int pindex;
    int i;

    if (!plist) return ERRPLUNDEF;

    /* Got a symbol table for this property list? */
    if (pl->pl_symtab) {

        /* Yes, compute hash of specified name */
        i = PListHashName(pl->pl_symtab, pname);

        /* Search hash collision list for matching name */
        for (pv = pl->pl_symtab->pt_hash[i]; pv; pv = pv->pv_next) {

            if (!strcmp(pname, pv->pv_name)) {

                /* Name match, get property index */
                pindex = pv->pv_pi;

                /* Return the value if requested */
                if (pvalue) *pvalue = (void *)(pv->pv_value);

                /* Return the type if requested */
                if (ptype) *ptype = (PList_t)(pv->pv_type);

                /* Return the property index */
                return pindex;
            }
        }
    }

    /* Error - specified property name is undefined */
    return ERRPLUNDEF;
}

/*
 * FUNCTION:    PListInitProp
 *
 * DESCRIPTION:
 *
 *      This function combines the functions of PListDefProp() and
 *      PListSetValue(), defining a new property and assigning it an
 *      initial value and optionally a type and/or a name.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *      pindex                  - a reserved property index, or zero
 *      pname                   - the new property name, or NULL
 *      pvalue                  - the new property value
 *      ptype                   - the new property type, or NULL
 *
 * RETURNS:
 *
 *      If successful, the property index (pindex) is returned as the
 *      function value.  Errors are indicated by a negative return code
 *      as defined in plist.h.
 */

NSAPI_PUBLIC int
PListInitProp(PList_t plist, int pindex, const char *pname,
              const void *pvalue, PList_t ptype)
{
    int rv;

    if (!plist) return ERRPLUNDEF;

    /* Create the property */
    rv = PListDefProp(plist, pindex, pname, PLFLG_USE_RES);
    if (rv > 0) {

        /* If that worked, set the value and type */
        rv = PListSetValue(plist, rv, pvalue, ptype);
    }

    return rv;
}

/*
 * FUNCTION:    PListNew
 *
 * DESCRIPTION:
 *
 *      This function creates a new property list, using the specified
 *      memory pool for allocating the internal data structures used to
 *      represent it.  If the mempool argument is NULL, the default
 *      memory pool is used.
 *
 * ARGUMENTS:
 *
 *      mempool                 - handle for a memory pool to be associated
 *                                with the new property list
 *
 * RETURNS:
 *
 *      If successful, the function return value is a handle for the new
 *      property list.  Otherwise NULL is returned.
 */

NSAPI_PUBLIC PList_t
PListNew(pool_handle_t *mempool)
{
    /* Just call PListCreate with default parameters */
    return PListCreate(mempool, 0, 0, 0);
}

/*
 * FUNCTION:    PListDestroy
 *
 * DESCRIPTION:
 *
 *      This function destroys a specified property list.  This means
 *      that any dynamic memory which was allocated as a result of calls
 *      to the property list API is freed to the memory pool from which
 *      it was allocated.  Property value data is not freed, nor are
 *      any property lists associated with property types.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 */

void
PListDestroy(PList_t plist)
{
    PListStruct_t *pl = (PListStruct_t *)plist;
    PLValueStruct_t **ppval;
    PLValueStruct_t *pv;
    int i;

    if (!plist) return;

    /* Free the property name symbol table if any */
    if (pl->pl_symtab) {
        pool_free(pl->pl_mempool, (void *)(pl->pl_symtab));
    }

    ppval = (PLValueStruct_t **)(pl->pl_ppval);

    /* Loop over the initialized property indices */
    for (i = 0; i < pl->pl_initpi; ++i) {

        /* Got a property here? */
        pv = ppval[i];
        if (pv) {

            /* Free the property name string if any */
            if (pv->pv_name) {
                pool_free(pv->pv_mempool, (void *)(pv->pv_name));
            }

            /* Free the property value structure */
            pool_free(pv->pv_mempool, (void *)pv);
        }
    }

    /* Free the array of pointers to property values */
    pool_free(pl->pl_mempool, (void *)ppval);

    /* Free the property list head */
    pool_free(pl->pl_mempool, (void *)pl);
}

/*
 * FUNCTION:    PListGetValue
 *
 * DESCRIPTION:
 *
 *      This function retrieves the value and type of the property with
 *      the property index given by pindex in the specified property
 *      list.  The pindex argument must specify the index of a defined
 *      property.  If the pvalue argument is non-NULL, it specifies a
 *      location in which to return the property value.  Similarly, if
 *      ptype is non-NULL, it specifies where the property list
 *      describing the property type is to be returned.  If a property
 *      has no value, the value returned for pvalue is NULL.  If a
 *      property has no type, the value returned for ptype is NULL. A
 *      property can have a value, a type, both, or neither.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *      pindex                  - the property index
 *      pvalue                  - property value return pointer
 *      ptype                   - property type return pointer
 *
 * RETURNS:
 *
 *      If successful, the property index (pindex) is returned as the
 *      function value.  Errors are indicated by a negative return code
 *      as defined in plist.h.
 */

NSAPI_PUBLIC int
PListGetValue(PList_t plist, int pindex, void **pvalue, PList_t *ptype)
{
    PListStruct_t *pl = (PListStruct_t *)plist;
    PLValueStruct_t **ppval;
    PLValueStruct_t *pv;

    if (!plist) return ERRPLUNDEF;

    ppval = (PLValueStruct_t **)(pl->pl_ppval);

    /* Check for valid property index */
    if ((pindex > 0) && (pindex <= pl->pl_initpi)) {

        /* Does the property exist? */
        pv = ppval[pindex - 1];
        if (pv) {

            /* Yes, return the value if requested */
            if (pvalue) *pvalue = (void *)(pv->pv_value);

            /* Return the type if requested */
            if (ptype) *ptype = (PList_t)(pv->pv_type);

            /* Successful return */
            return pindex;
        }
    }

    /* Error - invalid property index or non-existent property */
    return ERRPLINVPI;
}

/*
 * FUNCTION:    PListHash
 *
 * DESCRIPTION:
 *
 *      This function hashes a given string.
 *
 * ARGUMENTS:
 *
 *      string                  - pointer to the string to hash
 *
 * RETURNS:
 *
 *      The hash value is returned as the function value.
 */

unsigned int
PListHash(const char *string)
{
    unsigned int hashval = 0;           /* hash value */

    while (*string) {
        hashval = (hashval<<5) ^ (*string++ & 0x7f);
    }

    return hashval;
}

/*
 * FUNCTION:    PListHashName
 *
 * DESCRIPTION:
 *
 *      This function hashes a given property name for a specified
 *      symbol table.  It produces a value that can be used as an
 *      index in the pt_hash array associated with the symbol table.
 *
 * ARGUMENTS:
 *
 *      symtab                  - pointer to the symbol table
 *      pname                   - pointer to the property name string
 *
 * RETURNS:
 *
 *      The hash index is returned as the function value.
 */

int
PListHashName(PLSymbolTable_t *symtab, const char *pname)
{
    return PListHash(pname) % PLSIZENDX(symtab->pt_sizendx);
}

/*
 * FUNCTION:    PListNameProp
 *
 * DESCRIPTION:
 *
 *      This function assigns a name to a defined property with the
 *      property index, pindex.  If the property has an existing name,
 *      it will be replaced with the name specified by pname.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *      pindex                  - the property index
 *      pname                   - the new property name
 *
 * RETURNS:
 *
 *      If successful, the property index (pindex) is returned as the
 *      function value.  Errors are indicated by a negative return code
 *      as defined in plist.h.
 */

NSAPI_PUBLIC int
PListNameProp(PList_t plist, int pindex, const char *pname)
{
    PListStruct_t *pl = (PListStruct_t *)plist;
    PLValueStruct_t *pv;
    PLSymbolTable_t *pt;
    int i;

    if (!plist) return ERRPLUNDEF;

    pt = pl->pl_symtab;

    /* Check for valid property index */
    if ((pindex > 0) && (pindex <= pl->pl_initpi)) {

        /* Does the property exist? */
        pv = ((PLValueStruct_t **)(pl->pl_ppval))[pindex - 1];
        if (pv) {

            /* If it has a name already, unname it */
            if (pv->pv_name) {
                PLValueStruct_t **pvp;

                /* Get hash bucket index */
                i = PListHashName(pt, pv->pv_name);

                /* Seach hash collision list for this property */
                for (pvp = &pt->pt_hash[i];
                     *pvp; pvp = &(*pvp)->pv_next) {

                    if (*pvp == pv) {

                        /* Remove it from the list */
                        *pvp = pv->pv_next;
                        pt->pt_nsyms--;
                        break;
                    }
                }

                /* Free the current name string */
                pool_free(pv->pv_mempool, (void *)(pv->pv_name));
            }

            /* Got a new name? */
            if (pname) {

                /* Allocate/grow the symbol table as needed */
                pt = PListSymbolTable(pl);
                if (!pt) {
                    return ERRPLNOMEM;
                }

                /* Duplicate the name string */
                pv->pv_name = pool_strdup(pv->pv_mempool, (char *)pname);

                /* Add name to symbol table */
                i = PListHashName(pt, pname);
                pv->pv_next = pt->pt_hash[i];
                pt->pt_hash[i] = pv;
                pt->pt_nsyms++;
            }

            /* Successful return */
            return pindex;
        }
    }

    /* Error - invalid property index or non-existent property */
    return ERRPLINVPI;
}

/*
 * FUNCTION:    PListSetType
 *
 * DESCRIPTION:
 *
 *      This function sets the property type of the defined property
 *      with the property index, pindex.  The property list describing
 *      the property type is specified by ptype.  If ptype is NULL,
 *      the property type will be set to be undefined (NULL).
 *
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *      pindex                  - the property index
 *      ptype                   - the new property type, or NULL
 *
 * RETURNS:
 *
 *      If successful, the property index (pindex) is returned as the
 *      function value.  Errors are indicated by a negative return code
 *      as defined in plist.h.
 */

NSAPI_PUBLIC int
PListSetType(PList_t plist, int pindex, PList_t ptype)
{
    PListStruct_t *pl = (PListStruct_t *)plist;
    PLValueStruct_t **ppval;
    PLValueStruct_t *pv;

    if (!plist) return ERRPLUNDEF;

    ppval = (PLValueStruct_t **)(pl->pl_ppval);

    /* Check for valid property index */
    if ((pindex > 0) && (pindex <= pl->pl_initpi)) {

        /* Does the property exist? */
        pv = ppval[pindex - 1];
        if (pv) {

            /* Yes, set the new type */
            pv->pv_type = ptype;

            /* Successful return */
            return pindex;
        }
    }

    /* Error - invalid property index or non-existent property */
    return ERRPLINVPI;
}

/*
 * FUNCTION:    PListSetValue
 *
 * DESCRIPTION:
 *
 *      This function sets the value and optionally the type of a
 *      defined property in a given property list.  The pindex argument
 *      specifies the property index, which must be greater than zero.
 *      The ptype argument specifies a property list that describes the
 *      property type.  If ptype is NULL, the property type, if any, is
 *      unchanged by this function.  However, the property value is
 *      always set to the value given by pvalue.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *      pindex                  - the property index
 *      pvalue                  - the new property value
 *      ptype                   - the new property type, or NULL
 *
 * RETURNS:
 *
 *      If successful, the property index (pindex) is returned as the
 *      function value.  Errors are indicated by a negative return code
 *      as defined in plist.h.
 */

NSAPI_PUBLIC int
PListSetValue(PList_t plist, int pindex, const void *pvalue, PList_t ptype)
{
    PListStruct_t *pl = (PListStruct_t *)plist;
    PLValueStruct_t **ppval;
    PLValueStruct_t *pv;

    if (!plist) return ERRPLUNDEF;

    ppval = (PLValueStruct_t **)(pl->pl_ppval);

    /* Check for valid property index */
    if ((pindex > 0) && (pindex <= pl->pl_initpi)) {

        /* Does the property exist? */
        pv = ppval[pindex - 1];
        if (pv) {

            /* Yes, set the new value */
            pv->pv_value = (char *)pvalue;

            /* Set type if type is given */
            if (ptype) pv->pv_type = (PListStruct_t *)ptype;

            /* Successful return */
            return pindex;
        }
    }

    /* Error - invalid property index or non-existent property */
    return ERRPLINVPI;
}

/*
 * FUNCTION:    PListEnumerate
 *
 * DESCRIPTION:
 *
 *      This function walks through a specified property list
 *	calling a user supplied function with the property
 *	name and value as parameters.  
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *      user_func               - handle for the user function 
 */

NSAPI_PUBLIC void
PListEnumerate(PList_t plist, PListFunc_t *user_func, void *user_data)
{
    PListStruct_t *pl = (PListStruct_t *)plist;
    PLValueStruct_t **ppval;
    PLValueStruct_t *pv;
    int i;

    if (!plist) return;

    ppval = (PLValueStruct_t **)(pl->pl_ppval);

    /* Loop over the initialized property indices */
    for (i = 0; i < pl->pl_initpi; ++i) {

        /* Got a property here? */
        pv = ppval[i];
        if (pv) {
            (*user_func)(pv->pv_name, pv->pv_value, user_data);
        }

    }

}

/*
 * FUNCTION:    PListCreateDuplicate
 *
 * DESCRIPTION:
 *
 *      This function creates a new property list and returns a handle for
 *      it.  The source plist provides the new plists parameters.
 *
 * ARGUMENTS:
 *
 *	src_plist		- source plist to duplicate
 *      mempool                 - handle for a memory pool to be associated
 *                                with the new property list, only
 *				  used if flags is set to PLFLG_NEW_MPOOL
 *      flags                   - if PLFLG_NEW_MPOOL uses new_mempool
 *				  parameter
 *
 * RETURNS:
 *
 *      If successful, the function return value is a handle for the new
 *      property list.  Otherwise NULL is returned.
 */

static PList_t
PListCreateDuplicate(PList_t src_plist, pool_handle_t *new_mempool, int flags)
{
    PListStruct_t *plist;       /* pointer to property list structure */
    int i;
    pool_handle_t *mempool;

    mempool = (flags == PLFLG_NEW_MPOOL) ? new_mempool : src_plist->pl_mempool;

    plist = (PListStruct_t *)pool_malloc(mempool, sizeof(PListStruct_t));
    if (plist) {

        /* Initialize property list structure */
        plist->pl_mempool = mempool;
        plist->pl_symtab = NULL;
        plist->pl_maxprop = src_plist->pl_maxprop;
        plist->pl_resvpi = src_plist->pl_resvpi;
        plist->pl_initpi = src_plist->pl_initpi;
        plist->pl_lastpi = src_plist->pl_lastpi;

        /* Set initialize size for array of property value pointers */
        plist->pl_cursize = src_plist->pl_cursize; 

        /* Allocate the initial array of property value pointers */
        plist->pl_ppval = (pb_entry **)pool_malloc(mempool,
                                                   (plist->pl_cursize *
                                                    sizeof(PLValueStruct_t *)));
        if (!plist->pl_ppval) {

            /* Failed - insufficient memory */
            pool_free(mempool, (void *)plist);
            plist = NULL;
        }
        else {
            /* NULL out pointers in the reserved index range, if any */
            for (i = 0; i < plist->pl_lastpi; ++i) {
                plist->pl_ppval[i] = 0;
            }
        }
    }

    return (PList_t)plist;
}


/*
 * FUNCTION:    PListDuplicate
 *
 * DESCRIPTION:
 *
 *      This function duplicates a specified PList_t.  
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *      mempool                 - handle for a memory pool to be associated
 *                                with the new property list
 *      resvprop                - number of reserved property indices
 *      maxprop                 - maximum number of properties in list
 *                                (zero or negative imposes no limit)
 *      flags                   - unused, reserved, must be zero
 *
 * RETURNS:
 *
 *      If successful, the function return value is a handle for the new
 *      property list.  Otherwise NULL is returned.
 */

NSAPI_PUBLIC PList_t
PListDuplicate(PList_t plist, pool_handle_t *new_mempool, int flags)
{
    PListStruct_t *pl = (PListStruct_t *)plist;
    PLValueStruct_t **ppval;
    PLValueStruct_t *pv;
    int i;
    int rv = 0;
    PList_t new_plist;

    if (!plist) return NULL;

    new_plist = PListCreateDuplicate(plist, new_mempool, flags);
    if (new_plist == NULL) {
        return(NULL);
    }

    ppval = (PLValueStruct_t **)(pl->pl_ppval);

    /* Loop over the initialized property indices */
    for (i = 0; i < pl->pl_initpi; ++i) {

        /* Got a property here? */
        pv = ppval[i];
        if (pv) {
            /* Create the property */
            rv = PListDefProp(new_plist, i + 1, pv->pv_name, PLFLG_IGN_RES);
            if (rv > 0) {
        
                /* If that worked, set the value and type */
                rv = PListSetValue(new_plist, rv, pv->pv_value, pv->pv_type);
            }
        
            if ( rv <= 0 ) {
                PListDestroy(new_plist);
                return(NULL);
            }
        }

    }

    return(new_plist);
}

/*
 * FUNCTION:    PListGetPool
 *
 * DESCRIPTION:
 *
 *      This function returns the memory pool the PList is allocated from.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *
 * RETURNS:
 *
 *      The memory pool address, which can be NULL.
 */

NSAPI_PUBLIC pool_handle_t *
PListGetPool(PList_t plist)
{
    if (!plist) return NULL;

    return(plist->pl_mempool);
}

/*
 * FUNCTION:    PListGetFreeIndex
 *
 * DESCRIPTION:
 *
 *      This function returns an available property index.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *
 * RETURNS:
 *
 *      If successful, an available property index is returned as the
 *      function value.  Errors are indicated by a negative return code
 *      as defined in plist.h.
 */

int
PListGetFreeIndex(PListStruct_t *pl)
{
    PLValueStruct_t **ppval = (PLValueStruct_t **)(pl->pl_ppval);
    int wrapped;
    int i;

    /*
     * Look for a free property index, starting at pl_lastpi + 1.
     * (Note that i is the property index - 1)
     */
    for (wrapped = 0, i = pl->pl_lastpi; ;) {

        /* Are we in an initialized part of the array? */
        if (i < pl->pl_initpi) {

            /* Yes, use this index if it's free */
            if (ppval[i] == 0) break;

            /* Otherwise step to the next one */
            ++i;
        }
        else {

            /* Have we reached the end yet? */
            if (i < pl->pl_cursize) {

                /*
                 * We are above the highest initialized index, but
                 * still within the allocated size.  An index in
                 * this range can be used with no further checks.
                 */
                ppval[i] = 0;
            }
            else {

                /*
                 * It's looking like time to grow the array, but
                 * first go back and look for an unused, unreserved
                 * index that might have been freed.
                 */
                if (!wrapped) {

                    i = pl->pl_resvpi;
                    wrapped = 1;
                    continue;
                }

                /*
                 * Grow the array unless there is a specified maximum
                 * size and we've reached it.
                 */
                i = pl->pl_cursize;
                if (pl->pl_maxprop && (i > pl->pl_maxprop)) {

                    /* Error - property list is full */
                    return ERRPLFULL;
                }

                /* Increase planned size of list */
                int cursize = i + PLIST_DEFGROW;

                /* Reallocate the array of property value pointers */
                ppval = (PLValueStruct_t **)pool_realloc(pl->pl_mempool,
                                   (void *)ppval,
                                   (cursize * sizeof(PLValueStruct_t *)));
                if (!ppval) {

                    /* Error - insufficient memory */
                    return ERRPLNOMEM;
                }

                /* Initialize the first new entry and select it */
                ppval[i] = NULL;
                pl->pl_ppval = (pb_entry **)ppval;
                pl->pl_cursize = cursize;
            }

            /* Update the highest initialized index value */
            pl->pl_initpi = i + 1;
            break;
        }
    }

    /* Set the starting point for the next allocation */
    pl->pl_lastpi = i + 1;

    return i + 1;
}

/*
 * FUNCTION:    PListSymbolTable
 *
 * DESCRIPTION:
 *
 *      This function allocates or grows a property list's symbol table as
 *      needed.
 *
 * ARGUMENTS:
 *
 *      plist                   - handle for the property list
 *
 * RETURNS:
 *
 *      If successful, a pointer to the symbol table is returned as the
 *      function value.  Errors are indicated by a NULL return code.
 */

PLSymbolTable_t *
PListSymbolTable(PListStruct_t *pl)
{
    PLSymbolTable_t *pt;
    int i;

    pt = pl->pl_symtab;

    /* Is there a hash table? */
    if (!pl->pl_symtab) {

        /* No, create one */
        pt = (PLSymbolTable_t *)pool_calloc(pl->pl_mempool, 1, PLHASHSIZE(0));

        pl->pl_symtab = pt;
    }
    else {

        /* Is it time to grow the hash table? */
        i = PLSIZENDX(pt->pt_sizendx);
        if ((pt->pt_sizendx < PLMAXSIZENDX) && pt->pt_nsyms >= (i + i)) {

            PLSymbolTable_t *npt;

            /* Yes, allocate the new table */
            npt = (PLSymbolTable_t *)pool_calloc(pl->pl_mempool, 1,
                                                 PLHASHSIZE(pt->pt_sizendx+1));
            if (npt) {
                npt->pt_sizendx = pt->pt_sizendx + 1;
                npt->pt_nsyms = pt->pt_nsyms;

                /* Rehash all the names into the new table, preserving order */
                for (i = 0; i < PLSIZENDX(pt->pt_sizendx); ++i) {
                    /* While there are names at this hash index... */
                    while (pt->pt_hash[i]) {
                        PLValueStruct_t **pvp;
                        int j;

                        /* Find the last name at this hash index */
                        for (pvp = &pt->pt_hash[i]; (*pvp)->pv_next; pvp = &(*pvp)->pv_next);
                        
                        /* Move the name to the new table */
                        j = PListHashName(npt, (*pvp)->pv_name);
                        (*pvp)->pv_next = npt->pt_hash[j];
                        npt->pt_hash[j] = (*pvp);

                        /* Remove the name from the old table */
                        *pvp = NULL;
                    }
                }

                pl->pl_symtab = npt;

                /* Free the old symbol table */
                pool_free(pl->pl_mempool, (void *)pt);
                pt = npt;
            }
        }
    }

    return pl->pl_symtab;
}
