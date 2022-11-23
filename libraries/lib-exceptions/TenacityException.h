#ifndef __AUDACITY_EXCEPTION__
#define __AUDACITY_EXCEPTION__

/*!********************************************************************

 Tenacity: A Digital Audio Editor

 @file TenacityException.h
 @brief Declare abstract class TenacityException, some often-used subclasses, and @ref GuardedCall

 Paul Licameli
 Avery King
 **********************************************************************/

#include "MemoryX.h"
#include "../lib-strings/Internat.h"
#include <exception>
#include <functional>

#include "Internat.h"

//! A type of an exception
enum class ExceptionType
{
    Internal, //!< Indicates internal failure from Tenacity.
    BadUserAction, //!< Indicates that the user performed an action that is not allowed.
    BadEnvironment, //!< Indicates problems with environment, such as a full disk
};

/// Base class for exceptions specially processed by the application
/** Objects of this type can be thrown and caught in any thread, stored, and then
 * used by the main thread in later idle time to explain the error condition to
 * the user.
 * 
 * Additionally, unlike AudacityException, What() returns TranslatableString,
 * allowing for translation of an exception message. (This wraps around
 * std::exception::what()).
 */
class EXCEPTIONS_API TenacityException : public std::exception
{
  public:
    TenacityException();
    TenacityException(const char* what_arg);
    virtual ~TenacityException();

    //! Don't allow moves of this class or subclasses
    // see https://bugzilla.audacityteam.org/show_bug.cgi?id=2442
    TenacityException( TenacityException&& ) = delete;

    //! Disallow assignment
    TenacityException &operator = ( const TenacityException & ) = delete;


    /// Action to do in the main thread at idle time of the event loop.
    virtual void DelayedHandlerAction() = 0;

    /// Returns a C-style string containing the exception message
    virtual const char* what();

    static void EnqueueAction(
       std::exception_ptr pException,
       std::function<void(TenacityException*)> delayedHandler);

  protected:
    std::string m_WhatMsg;
};

//! Abstract AudacityException subclass displays a message, specified by further subclass
/*! At most one message will be displayed for each pass through the main event idle loop,
 no matter how many exceptions were caught. */
class EXCEPTIONS_API MessageBoxException /* not final */
   : public TenacityException
{
  //! Privatize the inherited function
  using TenacityException::DelayedHandlerAction;

  //! Do not allow subclasses to change behavior, except by overriding ErrorMessage().
  void DelayedHandlerAction() final;

  protected:
    //! If default-constructed with empty caption, it makes no message box.
    explicit MessageBoxException(
      ExceptionType exceptionType, //!< Exception type
      const TranslatableString &caption //!< Shown in message box's frame; not the actual message
    );

    ~MessageBoxException() override;

    MessageBoxException( const MessageBoxException& );

    //! %Format the error message for this exception.
    virtual TranslatableString ErrorMessage() const = 0;
    virtual wxString ErrorHelpUrl() const { return helpUrl; };

  private:
    TranslatableString caption; //!< Stored caption
    ExceptionType exceptionType; //!< Exception type

    mutable bool moved { false }; //!< Whether @c *this has been the source of a copy

  protected:
    mutable wxString helpUrl{ "" };
};

//! A MessageBoxException that shows a given, unvarying string.
class EXCEPTIONS_API SimpleMessageBoxException /* not final */
   : public MessageBoxException
{
public:
   explicit SimpleMessageBoxException(
      ExceptionType exceptionType,        //!< Exception type
      const TranslatableString &message_, //<! Message to show
      const TranslatableString &caption = XO("Message"), //<! Short caption in frame around message
      const wxString &helpUrl_ = "" // Optional URL for help.
   )
      : MessageBoxException { exceptionType, caption }
      , message{ message_ }
   {
      helpUrl = helpUrl_;
   }
   ~SimpleMessageBoxException() override;

   SimpleMessageBoxException( const SimpleMessageBoxException& ) = default;
   SimpleMessageBoxException &operator = (
      SimpleMessageBoxException && ) = delete;

   // Format a default, internationalized error message for this exception.
   virtual TranslatableString ErrorMessage() const override;

private:
   TranslatableString message; //!< Stored message
};


//! A default template parameter for @ref GuardedCall
struct DefaultDelayedHandlerAction
{
   void operator () (TenacityException *pException) const
   {
      if ( pException )
         pException->DelayedHandlerAction();
   }
};

//! A default template parameter for @ref GuardedCall<R>
/*! @tparam R return type from GuardedCall (or convertible to it) */
template <typename R> struct SimpleGuard
{
   explicit SimpleGuard(
      R value //!< The value to return from GurdedCall when an exception is handled
   )
      : m_value{ value } {}
   R operator () ( TenacityException * ) const { return m_value; }
   const R m_value;
};

//! Specialization of SimpleGuard, also defining a default value
template<> struct SimpleGuard<bool>
{
   explicit SimpleGuard(
      bool value //!< The value to return from @ref GaurdedCall when an exception is handled
   )
      : m_value{ value } {}
   bool operator () ( TenacityException * ) const { return m_value; }
   static SimpleGuard Default()
      { return SimpleGuard{ false }; }
   const bool m_value;
};

//! Specialization of SimpleGuard, also defining a default value
template<> struct SimpleGuard<void>
{
   SimpleGuard() {}
   void operator () ( TenacityException * ) const {}
   static SimpleGuard Default() { return {}; }
};

//! Convert a value to a handler function returning that value, suitable for @ref GuardedCall<R>
template < typename R >
SimpleGuard< R > MakeSimpleGuard( R value )
{ return SimpleGuard< R >{ value }; }

//! Convert a value to a no-op handler function, suitable for @ref GuardedCall<void>
inline SimpleGuard< void > MakeSimpleGuard() { return {}; }

/*!
  Executes a given function (typically a lamba), in any thread.
 
  If there is any exception, can invoke another given function as handler, which may rethrow that or
  another exception, but usually just returns the value for the GuardedCall.
 
  If AudacityException is handled, then it queues up a delayed handler action for execution later in
  the event loop at idle time, on the main thread; typically this informs the user of the error.

  The default delayed handler action is simply to invoke a method of the AudacityException, but this
  too can be specified otherwise by a third function.

  @tparam R Return type, defaulted to void, or else the only explicit template parameter
  @tparam F1 deduced type of body function; takes no arguments, returns @b R
  @tparam F2 deduced type of handler function, or defaulted to @ref SimpleGuard<R>;
  takes pointer to AudacityException, which is null when some other type of exception is caught;
  return value is converted to @b R
  @tparam F3 deduced type of delayed handler function, if a nondefault argument is given;
  takes pointer to AudacityException, return value is unused
 */
template <
   typename R = void,

   typename F1, // function object with signature R()

   typename F2 = SimpleGuard< R > // function object
      // with signature R( AudacityException * )
>
//! Execute some code on any thread; catch any AudacityException; enqueue error report on the main thread
R GuardedCall(
   const F1 &body, //!< typically a lambda
   const F2 &handler = F2::Default(), //!< default just returns false or void; see also @ref MakeSimpleGuard
   std::function<void(TenacityException*)> delayedHandler
      = DefaultDelayedHandlerAction{} /*!<called later in the main thread,
                                       passing it a stored exception; usually defaulted */
)
{
   try { return body(); }
   catch ( TenacityException &e ) {

      auto end = finally([&]{
         // At this point, e is the "current" exception, but not "uncaught"
         // unless it was rethrown by handler.  handler might also throw some
         // other exception object.
         if (!std::uncaught_exceptions()) {
            auto pException = std::current_exception(); // This points to e
            TenacityException::EnqueueAction(
               pException, std::move(delayedHandler));
         }
      });

      return handler( &e );
   }
   catch ( ... ) {
      return handler( nullptr );
   }
}

#endif
