/*
 Copyright (c) 2014, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "cinder/app/AppBasicMsw.h"
#include "cinder/app/AppImplMswBasic.h"
#include "cinder/app/PlatformMsw.h"
#include "cinder/Unicode.h"
#include "cinder/CinderAssert.h"

using namespace std;

namespace cinder { namespace app {

void AppBasicMsw::Settings::pushBackCommandLineArg( const std::string &arg )
{
	mCommandLineArgs.push_back( arg );
}

AppBasicMsw::AppBasicMsw()
{
	sInstance = this;

	const Settings *settings = dynamic_cast<Settings *>( sSettingsFromMain );
	CI_ASSERT( settings );

	// pull out app-level variables
	mConsoleWindowEnabled = settings->isConsoleWindowEnabled();
	enablePowerManagement( settings->isPowerManagementEnabled() ); // TODO: consider moving to common method

	Platform::get()->setExecutablePath( getAppPath() );
	mImpl.reset( new AppImplMswBasic( this, *settings ) );
}

// static
void AppBasicMsw::initialize( Settings *settings, const RendererRef &defaultRenderer, const char *title )
{
	AppBase::initialize( settings, defaultRenderer, title, 0, nullptr );

	// MSW sends it arguments as wide strings, so convert them to utf8 and store those in settings
	LPWSTR *szArglist;
	int nArgs;

	szArglist = ::CommandLineToArgvW( ::GetCommandLineW(), &nArgs );
	if( szArglist && nArgs ) {
		for( int i = 0; i < nArgs; ++i )
			settings->pushBackCommandLineArg( toUtf8( (char16_t*) szArglist[i] ) );
	}

	// Free memory allocated for CommandLineToArgvW arguments.
	::LocalFree( szArglist );
}

void AppBasicMsw::launch( const char *title, int argc, char * const argv[] )
{
	// allocate and redirect the console if requested
	if( mConsoleWindowEnabled ) {
		::AllocConsole();
		freopen( "CONIN$", "r", stdin );
		freopen( "CONOUT$", "w", stdout );
		freopen( "CONOUT$", "w", stderr );

		// tell msw platform that it should use std::cout for the console
		auto platformMsw = reinterpret_cast<PlatformMsw *>( Platform::get() );
		CI_ASSERT_MSG( platformMsw, "expected current Platform to be of type PlatformMsw" );

		platformMsw->directConsoleToCout( true );
	}

	mImpl->run();
}

WindowRef AppBasicMsw::createWindow( const Window::Format &format )
{
	return mImpl->createWindow( format );
}

void AppBasicMsw::quit()
{
	mImpl->quit();
}

float AppBasicMsw::getFrameRate() const
{
	return mImpl->getFrameRate();
}

void AppBasicMsw::setFrameRate( float frameRate )
{
	mImpl->setFrameRate( frameRate );
}

void AppBasicMsw::disableFrameRate()
{
	mImpl->disableFrameRate();
}

bool AppBasicMsw::isFrameRateEnabled() const
{
	return mImpl->isFrameRateEnabled();
}

fs::path AppBasicMsw::getAppPath() const
{
	return AppImplMsw::getAppPath();
}

WindowRef AppBasicMsw::getWindow() const
{
	return mImpl->getWindow();
}

WindowRef AppBasicMsw::getWindowIndex( size_t index ) const
{
	return mImpl->getWindowIndex( index );
}

size_t AppBasicMsw::getNumWindows() const
{
	return mImpl->getNumWindows();
}

WindowRef AppBasicMsw::getForegroundWindow() const
{
	return mImpl->getForegroundWindow();
}

void AppBasicMsw::hideCursor()
{
	AppImplMsw::hideCursor();
}

void AppBasicMsw::showCursor()
{
	AppImplMsw::showCursor();
}

ivec2 AppBasicMsw::getMousePos() const
{
	POINT point;
	::GetCursorPos( &point );

	return ivec2( point.x, point.y );
}

} } // namespace cinder::app