/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"
#include "SessionService.h"

#include "Config.h"

#include "HelioTheme.h"
#include "App.h"

#include "ProgressTooltip.h"
#include "SuccessTooltip.h"
#include "FailTooltip.h"

// Try to update our sliding session and reload user profile after 5 seconds
#define UPDATE_SESSION_TIMEOUT_MS (1000 * 5)

SessionService::SessionService() :
    userProfile({})
{
    // The logic goes like this:
    // If we have a token, request user profile,
    // which will either fail with expired token (do nothing)
    // or return a profile - then we check if token was updated
    // more than 1 day ago, start update token thread

    this->startTimer(UPDATE_SESSION_TIMEOUT_MS);
}

String SessionService::getApiToken()
{
    return Config::get(Serialization::Api::sessionLastToken, {});
}

bool SessionService::isLoggedIn()
{
    return SessionService::getApiToken().isNotEmpty();
}

const UserProfile & SessionService::getUserProfile() const noexcept
{
    return this->userProfile;
}

void SessionService::signIn(const String &login, const String &passwordHash, AuthCallback callback)
{
    if (this->loginCallback != nullptr)
    {
        jassertfalse; // You should never hit this line
        callback(false, { "Login is in progress" });
        return;
    }

    this->loginCallback = callback;
    this->getThreadFor<SignInThread>()->signIn(this, login, passwordHash);
}

void SessionService::signOut()
{
    // TODO: need to erase token on server?
    Config::set(Serialization::Api::sessionLastUpdateTime, Time::getCurrentTime().toISO8601(true));
    Config::set(Serialization::Api::sessionLastToken, {});
    this->userProfile.reset();
}

//===----------------------------------------------------------------------===//
// Updating session
//===----------------------------------------------------------------------===//

void SessionService::timerCallback()
{
    this->stopTimer();
    if (SessionService::getApiToken().isNotEmpty())
    {
        this->getThreadFor<RequestUserProfileThread>()->requestUserProfile(this);
    }
}

//===----------------------------------------------------------------------===//
// RequestUserProfileThread::Listener
//===----------------------------------------------------------------------===//

void SessionService::requestProfileOk(const UserProfile profile)
{
    this->userProfile = profile;

    const String lastSessionToken = SessionService::getApiToken();
    const Time nowMinusHalfDay = Time::getCurrentTime() - RelativeTime::hours(12);
    const Time lastSessionUpdateTime =
        Time::fromISO8601(Config::get(Serialization::Api::sessionLastUpdateTime));

    if (lastSessionToken.isNotEmpty() && lastSessionUpdateTime < nowMinusHalfDay)
    {
        this->getThreadFor<TokenUpdateThread>()->updateToken(this, lastSessionToken);
    }
}

void SessionService::requestProfileFailed(const Array<String> &errors)
{
    this->userProfile.reset();
}

//===----------------------------------------------------------------------===//
// SignInThread::Listener
//===----------------------------------------------------------------------===//

void SessionService::signInOk(const String &userEmail, const String &newToken)
{
    if (this->loginCallback != nullptr)
    {
        this->loginCallback(true, {});
        this->loginCallback = nullptr;
    }

    this->getThreadFor<RequestUserProfileThread>()->requestUserProfile(this);
}

void SessionService::signInFailed(const Array<String> &errors)
{
    Logger::writeToLog("Login failed: " + errors.getFirst());

    if (this->loginCallback != nullptr)
    {
        this->loginCallback(false, errors);
        this->loginCallback = nullptr;
    }
}

//===----------------------------------------------------------------------===//
// SignUpThread::Listener
//===----------------------------------------------------------------------===//

void SessionService::signUpOk(const String &userEmail, const String &newToken)
{
    if (this->registrationCallback != nullptr)
    {
        this->registrationCallback(true, {});
        this->registrationCallback = nullptr;
    }
}

void SessionService::signUpFailed(const Array<String> &errors)
{
    Logger::writeToLog("Registration failed: " + errors.getFirst());

    if (this->registrationCallback != nullptr)
    {
        this->registrationCallback(false, errors);
        this->registrationCallback = nullptr;
    }
}

//===----------------------------------------------------------------------===//
// TokenUpdateThread::Listener
//===----------------------------------------------------------------------===//

void SessionService::tokenUpdateOk(const String &newToken)
{
    Config::set(Serialization::Api::sessionLastToken, newToken);
    Config::set(Serialization::Api::sessionLastUpdateTime, Time::getCurrentTime().toISO8601(true));

    // Token update happens most often once a day
    // At this point we're safe to launch updates check:
    // TODO
}

void SessionService::tokenUpdateFailed(const Array<String> &errors)
{
    Config::set(Serialization::Api::sessionLastToken, {});
    this->userProfile.reset();
}

void SessionService::tokenUpdateNoResponse()
{
    // In case of connection error, we should not erase the token
    this->userProfile.reset();
}
