// @(#) $Id$

#include "tao/DiffServPolicy/DiffServPolicy.h"
#include "tao/DiffServEndpointPolicy/DiffServPolicy_ORBInitializer.h"
#include "tao/ORB_Core.h"
#include "tao/ORBInitializer_Registry.h"
#include "tao/PI/PI.h"

ACE_RCSID (DiffServPolicy, DiffServPolicy, "$Id$")

TAO_BEGIN_VERSIONED_NAMESPACE_DECL

int
TAO_DiffServPolicy_Initializer::init (void)
{
  PortableInterceptor::ORBInitializer_ptr temp_orb_initializer =
  PortableInterceptor::ORBInitializer::_nil ();
  PortableInterceptor::ORBInitializer_var orb_initializer;

  try
    {
      /// Register the EndpointPolicy ORBInitializer.
      ACE_NEW_THROW_EX (temp_orb_initializer,
                        TAO_DiffServPolicy_ORBInitializer,
                        CORBA::NO_MEMORY (
                          CORBA::SystemException::_tao_minor_code (
                            TAO::VMCID,
                            ENOMEM),
                          CORBA::COMPLETED_NO));

      orb_initializer = temp_orb_initializer;

      PortableInterceptor::register_orb_initializer (orb_initializer.in ());
    }
  catch (const ::CORBA::Exception& ex)
    {
      if (TAO_debug_level > 0)
        {
          ex._tao_print_exception ("Caught exception:");
        }
      return -1;
    }

  return 0;
}

TAO_END_VERSIONED_NAMESPACE_DECL
