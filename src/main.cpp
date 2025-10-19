/**
 * BetterSave - Cloud Save Manager for Geometry Dash
 * Created by: sidastuff
 * 
 * A comprehensive save management system with Firebase cloud backup,
 * secure authentication, and cross-device synchronization.
 * 
 * Copyright © 2024 sidastuff. All rights reserved.
 */

/**
 * Include the Geode headers.
 */
#include <Geode/Geode.hpp>
#include "LoginPopup.hpp"
#include "SaveManagerPopup.hpp"
#include "LogsViewerPopup.hpp"
#include "FirebaseAuth.hpp"
#include "BetterSaveLogger.hpp"
#include "AutoBackupScheduler.hpp"
#include <chrono>

/**
 * Brings cocos2d and all Geode namespaces to the current scope.
 */
using namespace geode::prelude;

// Global auto-backup scheduler hook
#include <Geode/modify/CCScheduler.hpp>
class $modify(BetterSaveScheduler, CCScheduler) {
	void update(float dt) {
		CCScheduler::update(dt);
		
		// Check auto-backup every frame (has internal timing logic)
		static float checkInterval = 0.0f;
		checkInterval += dt;
		
		// Check every 30 seconds
		if (checkInterval >= 30.0f) {
			checkInterval = 0.0f;
			AutoBackupScheduler::get()->checkAndBackup();
		}
	}
};

/**
 * `$modify` lets you extend and modify GD's classes.
 * To hook a function in Geode, simply $modify the class
 * and write a new function definition with the signature of
 * the function you want to hook.
 *
 * Here we use the overloaded `$modify` macro to set our own class name,
 * so that we can use it for button callbacks.
 *
 * Notice the header being included, you *must* include the header for
 * the class you are modifying, or you will get a compile error.
 *
 * Another way you could do this is like this:
 *
 * struct MyMenuLayer : Modify<MyMenuLayer, MenuLayer> {};
 */
#include <Geode/modify/MenuLayer.hpp>
class $modify(MyMenuLayer, MenuLayer) {
	/**
	 * Typically classes in GD are initialized using the `init` function, (though not always!),
	 * so here we use it to add our own button to the bottom menu.
	 *
	 * Note that for all hooks, your signature has to *match exactly*,
	 * `void init()` would not place a hook!
	*/
	bool init() {
		/**
		 * We call the original init function so that the
		 * original class is properly initialized.
		 */
		if (!MenuLayer::init()) {
			return false;
		}

		/**
		 * You can use methods from the `geode::log` namespace to log messages to the console,
		 * being useful for debugging and such. See this page for more info about logging:
		 * https://docs.geode-sdk.org/tutorials/logging
		*/
		log::debug("Hello from my MenuLayer::init hook! This layer has {} children.", this->getChildrenCount());

		/**
		 * See this page for more info about buttons
		 * https://docs.geode-sdk.org/tutorials/buttons
		 * Using folder icon - represents save data management
		*/
		auto myButton = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png"),
			this,
			/**
			 * Here we use the name we set earlier for our modify class.
			*/
			menu_selector(MyMenuLayer::onMyButton)
		);

		/**
		 * Here we access the `bottom-menu` node by its ID, and add our button to it.
		 * Node IDs are a Geode feature, see this page for more info about it:
		 * https://docs.geode-sdk.org/tutorials/nodetree
		*/
		auto menu = this->getChildByID("bottom-menu");
		menu->addChild(myButton);

		/**
		 * The `_spr` string literal operator just prefixes the string with
		 * your mod id followed by a slash. This is good practice for setting your own node ids.
		*/
		myButton->setID("my-button"_spr);

		/**
		 * We update the layout of the menu to ensure that our button is properly placed.
		 * This is yet another Geode feature, see this page for more info about it:
		 * https://docs.geode-sdk.org/tutorials/layouts
		*/
		menu->updateLayout();

		/**
		 * We return `true` to indicate that the class was properly initialized.
		 */
		return true;
	}

	/**
	 * Add keyboard shortcut handling
	 * Alt+G for backup, or Press B key 3 times quickly
	 */
	void keyDown(cocos2d::enumKeyCodes key) override {
		// Call original
		MenuLayer::keyDown(key);
		
		// Check for Alt+G shortcut
		if (key == cocos2d::KEY_G) {
			#ifdef GEODE_IS_WINDOWS
				// Check if Alt key is pressed (VK_MENU is Alt key on Windows)
				if (GetAsyncKeyState(VK_MENU) & 0x8000) {
					BetterSaveLogger::get()->info("Keyboard", "Backup shortcut triggered (Alt+G)");
					this->triggerManualBackup();
					return;
				}
			#elif defined(GEODE_IS_MACOS)
				// On Mac, check for Option key (same as Alt)
				if (CCDirector::sharedDirector()->getKeyboardDispatcher()->getAltKeyPressed()) {
					BetterSaveLogger::get()->info("Keyboard", "Backup shortcut triggered (Alt+G)");
					this->triggerManualBackup();
					return;
				}
			#endif
		}
		
		// Fallback shortcut: Press 'B' key 3 times within 1 second to trigger backup
		static int bPressCount = 0;
		static auto lastBPress = std::chrono::system_clock::now();
		
		if (key == cocos2d::KEY_B) {
			auto now = std::chrono::system_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBPress);
			
			if (elapsed.count() < 1000) {
				// Within 1 second of last press
				bPressCount++;
				if (bPressCount >= 3) {
					BetterSaveLogger::get()->info("Keyboard", "Backup shortcut triggered (BBB)");
					this->triggerManualBackup();
					bPressCount = 0;
				}
			} else {
				// Reset if too much time passed
				bPressCount = 1;
			}
			lastBPress = now;
		}
	}
	
	void triggerManualBackup() {
		// Check if logged in
		if (!FirebaseAuth::get()->isLoggedIn()) {
			FLAlertLayer::create("Not Logged In", 
				"You must be logged in to backup your save data.", 
				"OK")->show();
			return;
		}
		
		// Trigger the upload process
		BetterSaveLogger::get()->info("Backup", "Manual backup triggered via keyboard shortcut");
		
		// Create a temporary SaveManagerPopup to use its upload function
		auto popup = SaveManagerPopup::create();
		popup->uploadSaveData();
		
		// Show notification
		Notification::create("BetterSave: Backup Started", NotificationIcon::Info, 3.0f)->show();
	}

	/**
	 * This is the callback function for the button we created earlier.
	 * Opens the BetterSave login/signup popup or Save Manager if already logged in.
	 * Also attempts auto-login with saved credentials on first click.
	*/
	void onMyButton(CCObject*) {
		// Try auto-login first if not already logged in
		if (!FirebaseAuth::get()->isLoggedIn()) {
			// Attempt to load saved credentials
			FirebaseAuth::get()->tryAutoLogin([](bool success, const std::string& message) {
				if (success) {
					// Auto-login successful, show save manager
					SaveManagerPopup::create()->show();
				} else {
					// No saved credentials, show login popup
					LoginPopup::create()->show();
				}
			});
		} else {
			// Already logged in, show save manager directly
			SaveManagerPopup::create()->show();
		}
	}
};