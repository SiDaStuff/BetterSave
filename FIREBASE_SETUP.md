# Firebase Security Rules Setup for BetterSave

This guide explains how to set up and configure Firebase security rules for your BetterSave project.

---

## 📋 Prerequisites

- Firebase project created (https://console.firebase.google.com)
- Firebase Realtime Database enabled
- Firebase Authentication enabled with Email/Password provider

---

## 🔒 Security Rules Files

### 1. `firebase-rules.json` (Recommended)
**Balanced security with good user experience**

Features:
- ✅ User authentication required
- ✅ Users can only access their own data
- ✅ Validates data structure (saveData, chunks)
- ✅ Chunk count limit: 1000 max (allows ~200MB saves)
- ✅ Chunk size limit: 500KB max
- ✅ Chunk ID validation (gm0-gm9999, ll0-ll9999)
- ✅ Prevents unauthorized fields
- ⚠️ No timestamp validation (good for clock skew tolerance)

**Use this if:** You want solid security without strict timestamp checks.

### 2. `firebase-rules-strict.json` (Maximum Security)
**Enhanced security with additional validations**

Features:
- ✅ All features from basic rules
- ✅ Stricter chunk count limit: 500 max (allows ~100MB saves)
- ✅ Timestamp validation (must be recent)
- ✅ Hex validation (ensures data is properly encoded)
- ✅ Chunk ID limit: 0-999 (stricter than basic)
- ✅ Optional metadata tracking (uploads, downloads)
- ✅ Optional uploadedBy field for audit

**Use this if:** You want maximum security and have reliable server time sync.

---

## 🚀 How to Apply Rules

### Method 1: Firebase Console (Recommended)

1. Go to [Firebase Console](https://console.firebase.google.com)
2. Select your project (`gdbettersave`)
3. Navigate to **Realtime Database** → **Rules** tab
4. Copy the contents of `firebase-rules.json` (or `firebase-rules-strict.json`)
5. Paste into the rules editor
6. Click **Publish**

### Method 2: Firebase CLI

```bash
# Install Firebase CLI
npm install -g firebase-tools

# Login to Firebase
firebase login

# Initialize Firebase in your project directory
firebase init database

# Copy your chosen rules file
cp firebase-rules.json database.rules.json

# Deploy the rules
firebase deploy --only database
```

---

## 🔍 Rules Explanation

### User Authentication
```json
".read": "auth != null && auth.uid == $userId"
".write": "auth != null && auth.uid == $userId"
```
- Only authenticated users can access data
- Users can only read/write their own data path
- Prevents unauthorized access to other users' saves

### Data Structure Validation

#### SaveData Metadata
```json
"saveData": {
  "gmChunks": 0-1000,      // Number of GameManager chunks
  "llChunks": 0-1000,      // Number of LocalLevels chunks
  "timestamp": number,     // Upload timestamp
  "deviceInfo": "string"   // Optional device info
}
```

#### Chunk Data
```json
"chunks": {
  "gm0", "gm1", "gm2", ... // GameManager chunks
  "ll0", "ll1", "ll2", ... // LocalLevels chunks
  
  Each chunk contains:
  {
    "d": "hexadecimaldata..." // Hex-encoded save data
  }
}
```

### Size Limits

| Item | Limit | Reason |
|------|-------|--------|
| Chunk Count | 500-1000 | Prevents abuse, typical save ~50-200 chunks |
| Chunk Size | 500KB | Allows 200KB data + hex overhead |
| Device Info | 256 chars | Reasonable device name length |
| Chunk ID | gm/ll + 1-4 digits | Matches app's naming pattern |

---

## 🧪 Testing Your Rules

### Test with Firebase Console Simulator

1. Go to **Realtime Database** → **Rules** tab
2. Click **Rules Playground**
3. Test these scenarios:

**Test 1: Authenticated user reads own data**
```
Type: Read
Location: /users/testuid123/saveData
Auth: Authenticated (uid: testuid123)
Expected: ✅ Allow
```

**Test 2: User tries to read another user's data**
```
Type: Read
Location: /users/otheruid456/saveData
Auth: Authenticated (uid: testuid123)
Expected: ❌ Deny
```

**Test 3: Unauthenticated access**
```
Type: Read
Location: /users/testuid123/saveData
Auth: Unauthenticated
Expected: ❌ Deny
```

**Test 4: Valid chunk write**
```
Type: Write
Location: /users/testuid123/chunks/gm0
Auth: Authenticated (uid: testuid123)
Data: {"d": "48656c6c6f"}
Expected: ✅ Allow
```

**Test 5: Invalid chunk ID**
```
Type: Write
Location: /users/testuid123/chunks/invalid
Auth: Authenticated (uid: testuid123)
Data: {"d": "48656c6c6f"}
Expected: ❌ Deny
```

---

## 🎯 Best Practices

### 1. Use HTTPS Only
Firebase automatically enforces HTTPS, but ensure your app uses secure connections.

### 2. Keep Auth Tokens Secure
```cpp
// In your app: Never log or expose auth tokens
FirebaseAuth::get()->getIdToken(); // Use securely
```

### 3. Monitor Usage
- Check Firebase Console → Realtime Database → Usage tab
- Set up billing alerts for unexpected spikes
- Monitor for unusual access patterns

### 4. Regular Security Audits
- Review Firebase Console → Authentication → Users
- Check for suspicious accounts
- Monitor data size per user

### 5. Rate Limiting (Optional)
Firebase doesn't have built-in rate limiting in rules. Consider:
- App-level throttling for uploads
- Firebase Functions for server-side rate limiting
- Cloud Functions to track and limit requests

---

## 📊 Data Size Estimates

| Save Size | Chunks (200KB each) | Storage Used |
|-----------|---------------------|--------------|
| 10 MB | ~50 chunks | ~20 MB (2x for hex) |
| 20 MB | ~100 chunks | ~40 MB |
| 50 MB | ~250 chunks | ~100 MB |
| 100 MB | ~500 chunks | ~200 MB |

**Firebase Free Tier:**
- 1 GB stored data
- 10 GB/month downloads
- 50K concurrent connections

---

## 🔧 Troubleshooting

### "Permission Denied" Error

**Cause:** User not authenticated or trying to access another user's data

**Solution:**
```cpp
// Ensure user is logged in
if (!FirebaseAuth::get()->isLoggedIn()) {
    // Show login popup
}

// Ensure using correct userId in URLs
std::string userId = FirebaseAuth::get()->getUserId();
```

### "Validation Failed" Error

**Cause:** Data doesn't match validation rules

**Common issues:**
1. Chunk ID wrong format (use `gm0`, `gm1`, not `chunk0`)
2. Missing required fields in saveData
3. Chunk size exceeds limit
4. Invalid characters in hex data (strict rules only)

**Solution:** Check your data format matches the rules schema.

### Token Expired

**Cause:** Auth token expired (tokens expire after 1 hour)

**Solution:**
```cpp
// Refresh token before major operations
FirebaseAuth::get()->refreshToken();
```

---

## 🔐 Security Checklist

- [ ] Rules deployed to Firebase
- [ ] Tested with Firebase Console simulator
- [ ] Email/Password authentication enabled
- [ ] Users can only access their own data
- [ ] Chunk size limits enforced
- [ ] Invalid data rejected
- [ ] Monitoring set up
- [ ] Backup strategy in place

---

## 📝 Migrating from Old Rules

If you had previous rules deployed:

1. **Backup current data** (export from Firebase Console)
2. Copy users' data structure: `users/{uid}/saveData` and `users/{uid}/chunks`
3. Deploy new rules
4. Test with a test account first
5. Monitor for any errors in Firebase Console → Database → Usage

---

## 🆘 Support

If you encounter issues:

1. Check Firebase Console logs
2. Verify auth tokens are valid
3. Test rules in simulator
4. Check app logs with BetterSave's logging system
5. Review this documentation

---

## 📚 Additional Resources

- [Firebase Security Rules Documentation](https://firebase.google.com/docs/database/security)
- [Firebase Auth Documentation](https://firebase.google.com/docs/auth)
- [Firebase Realtime Database Best Practices](https://firebase.google.com/docs/database/usage/best-practices)

---

**Created for BetterSave by sidastuff**

