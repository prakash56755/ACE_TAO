// -*- C++ -*-
// $Id$

#include "TAO_UTO.h"
#include "TAO_TIO.h"
#include "ace/OS.h"

// Constructor.

TAO_UTO::TAO_UTO (TimeBase::TimeT time,
                  TimeBase::InaccuracyT inaccuracy,
                  TimeBase::TdfT tdf)
{

  this->attr_utc_time_.time = time;

  // Extract the lower 32 bits in the inacclo.
  this->attr_utc_time_.inacclo = (CORBA::ULong) ACE_U64_TO_U32 (inaccuracy);

  // Extract the lower 16 bits of the remaining bits. 'And'ing with 0xFFFF
  // is only a sanity check.

#if defined (ACE_LACKS_U_LONGLONG_T)
  this->attr_utc_time_.inacchi = 0;
#else
  this->attr_utc_time_.inacchi = ACE_static_cast (CORBA::UShort, (inaccuracy >> 32) & 0xFFFF);

#endif /* ACE_LACKS_U_LONGLONG_T */

  this->attr_utc_time_.tdf = tdf;

}

// Destructor.

TAO_UTO::~TAO_UTO (void)
{
}

// Get Method for the readonly attribute time.

TimeBase::TimeT
TAO_UTO::time (CORBA::Environment &)
{
  return attr_utc_time_.time;
}

// Get method for the readonly attribute inaccuracy.

TimeBase::InaccuracyT
TAO_UTO::inaccuracy (CORBA::Environment &)
{
  // Construct the Inaccuracy from the
  // inacchi and inacclo.

  TimeBase::InaccuracyT inaccuracy = attr_utc_time_.inacchi;
  inaccuracy <<= 32;
  inaccuracy |= attr_utc_time_.inacclo;
  return inaccuracy;
}

// Get method for the readonly attribute tdf.

TimeBase::TdfT
TAO_UTO::tdf (CORBA::Environment &)
{
  return attr_utc_time_.tdf;
}

// Get method for the readonly attribute utc_time.

TimeBase::UtcT
TAO_UTO::utc_time (CORBA::Environment &)
{
  return attr_utc_time_;
}

// Absolute time = Relative time + Base time.  ?? Find out more about
// the Base Time, UTC and Distributed Time Sync. Algos. [3].

CosTime::UTO_ptr
TAO_UTO::absolute_time (CORBA::Environment &)
{
  return 0;
}

// Compares the time contained in the object with the time in the
// supplied uto according to the supplied comparison type.

CosTime::TimeComparison
TAO_UTO::compare_time (CosTime::ComparisonType comparison_type,
                       CosTime::UTO_ptr uto,
                       CORBA::Environment &)
{
  TAO_TRY
    {
      if (comparison_type == CosTime::MidC)
        {
          if (this->time (TAO_TRY_ENV) == uto->time (TAO_TRY_ENV))
            {
              TAO_CHECK_ENV;
              return CosTime::TCEqualTo;
            }
          else if (this->time (TAO_TRY_ENV) > uto->time (TAO_TRY_ENV))
            {
              TAO_CHECK_ENV;
              return CosTime::TCGreaterThan;
            }
          else
            return CosTime::TCLessThan;
        }
      else if (this->time (TAO_TRY_ENV) == uto->time (TAO_TRY_ENV))
        {
          TAO_CHECK_ENV;
          if (this->inaccuracy (TAO_TRY_ENV) == 0 && uto->inaccuracy (TAO_TRY_ENV) == 0)
            {
              TAO_CHECK_ENV;
              return CosTime::TCEqualTo;
            }
        }
      else
        {
          if (this->time (TAO_TRY_ENV) > uto->time (TAO_TRY_ENV))
            {
              TAO_CHECK_ENV;
              if (this->time (TAO_TRY_ENV) - this->inaccuracy (TAO_TRY_ENV)
                  > uto->time (TAO_TRY_ENV) - uto->inaccuracy (TAO_TRY_ENV))
                {
                  TAO_CHECK_ENV;
                  return CosTime::TCGreaterThan;
                }
            }
          else if (this->time (TAO_TRY_ENV) + this->inaccuracy (TAO_TRY_ENV)
                   < uto->time (TAO_TRY_ENV) - uto->inaccuracy (TAO_TRY_ENV))

            {
              TAO_CHECK_ENV;
              return CosTime::TCLessThan;
            }
        }

    }
  TAO_CATCHANY
    {
      TAO_TRY_ENV.print_exception ("Exception:");
    }
  TAO_ENDTRY;

  return CosTime::TCIndeterminate;
}

// Returns a TIO representing the time interval between the time in
// the object and the time in the UTO passed as a parameter. The
// interval returned is the interval between the mid-points of the two
// UTOs. Inaccuracies are ignored.  Note the result of this operation
// is meaningless if the base times of UTOs are different.

CosTime::TIO_ptr
TAO_UTO::time_to_interval (CosTime::UTO_ptr uto,
                           CORBA::Environment &TAO_IN_ENV)
{
  TAO_TIO *tio = 0;

  // @@ Vishal:  This code doesn't make any sense to me also.

  TAO_TRY
    {
      if (this->time (TAO_TRY_ENV) > uto->time (TAO_TRY_ENV))
        ACE_NEW_THROW_RETURN (tio,
                              TAO_TIO (uto->time (TAO_TRY_ENV),
                                       this->time (TAO_TRY_ENV)),
                              CORBA::NO_MEMORY (CORBA::COMPLETED_NO),
                              CosTime::TIO::_nil ());
      else
        ACE_NEW_THROW_RETURN (tio,
                              TAO_TIO (this->time (TAO_TRY_ENV),
                                       uto->time (TAO_TRY_ENV)),
                              CORBA::NO_MEMORY (CORBA::COMPLETED_NO),
                              CosTime::TIO::_nil ());
      TAO_CHECK_ENV;

    }
  TAO_CATCHANY
    {
      TAO_TRY_ENV.print_exception ("Exception:");
      return CosTime::TIO::_nil ();
    }
  TAO_ENDTRY;

  return tio->_this ();
}

  // Returns a TIO object representing the error interval around the
  // time value in the UTO.

CosTime::TIO_ptr
TAO_UTO::interval (CORBA::Environment &TAO_IN_ENV)
{
  TAO_TIO *tio = 0;

  TAO_TRY
    {
      TimeBase::TimeT lower =
        this->time (TAO_TRY_ENV) - this->inaccuracy (TAO_TRY_ENV);
      TAO_CHECK_ENV;

      TimeBase::TimeT upper =
        this->time (TAO_TRY_ENV) + this->inaccuracy (TAO_TRY_ENV);
      TAO_CHECK_ENV;

      // @@ Vishal:  So is this code.  You can't throw_return from a try
      // block.
      ACE_NEW_THROW_RETURN (tio,
                            TAO_TIO (lower,
                                     upper),
                            CORBA::NO_MEMORY (CORBA::COMPLETED_NO),
                            CosTime::TIO::_nil ());
      TAO_CHECK_ENV;
    }
  TAO_CATCHANY
    {
      TAO_TRY_ENV.print_exception ("Exception:");
      return CosTime::TIO::_nil ();
    }
  TAO_ENDTRY;

  return tio->_this ();
}
