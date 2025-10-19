# Security Policy - BetterSave

## 🔒 Security Overview

BetterSave takes security seriously. This document outlines our security measures, best practices, and how to report vulnerabilities.

---

## 🛡️ Security Features

### 1. Authentication
- **Firebase Authentication** with email/password
- Secure token-based authentication (JWT)
- Tokens automatically refresh (1-hour expiration)
- Credentials stored locally with encryption
- No passwords stored in plain text

### 2. Data Protection
- **User isolation**: Each user can only access their own data
- **Firebase Security Rules** enforce server-side access control
- **Hex encoding** for binary data transmission
- **HTTPS only** for all network communications
- **Token-based API access** (auth tokens in URL params)

### 3. Data Integrity
- **File verification** before upload/download
- **Chunk validation** during parallel transfers
- **Size limits** to prevent abuse
- **CRC32 checksums** for save file validation

### 4. Local Security
- Auth tokens stored in Geode's secure save directory
- Credentials encrypted in local storage
- Logs don't contain sensitive information
- No plaintext password storage

---

## 🔐 Firebase Security Rules

### Current Implementation

**Standard Rules** (`firebase-rules.json`):
```json
- User authentication required
- Users isolated to their own data path
- Chunk count limit: 1000 max
- Chunk size limit: 500KB max
- Data structure validation
- Prevents unauthorized fields
```

**Strict Rules** (`firebase-rules-strict.json`):
```json
- All standard protections plus:
- Timestamp validation (prevents replay attacks)
- Hex data validation (ensures proper encoding)
- Stricter limits (500 chunk max)
- Optional audit fields
```

### Best Practices

1. **Deploy rules immediately** after setting up Firebase
2. **Test rules** using Firebase Console simulator
3. **Monitor access** in Firebase Console
4. **Update rules** when data structure changes
5. **Review regularly** for security improvements

---

## 🚨 Known Limitations

### Client-Side Mod
- BetterSave runs as a Geometry Dash mod (client-side)
- Source code is visible and can be decompiled
- **Mitigation**: All security enforced server-side via Firebase Rules

### Token Exposure
- Auth tokens sent in URL parameters (Firebase REST API)
- Tokens visible in network traffic (though HTTPS encrypted)
- **Mitigation**: Tokens expire after 1 hour, server-side rules prevent abuse

### Local File Access
- Mod has access to local save files (required for functionality)
- **Mitigation**: This is intentional and necessary for save backup/restore

### No Rate Limiting
- Firebase Realtime Database rules don't support rate limiting
- **Mitigation**: Consider implementing Cloud Functions for rate limiting

---

## 🔍 Security Best Practices for Users

### 1. Account Security
- ✅ Use a strong, unique password
- ✅ Don't share your account credentials
- ✅ Log out from shared devices
- ⚠️ No password recovery implemented (use memorable passwords!)

### 2. Save Data Safety
- ✅ Keep local backups of important saves
- ✅ Don't upload corrupted save files
- ✅ Verify downloads in a test environment first
- ✅ Check logs for any suspicious activity

### 3. Device Security
- ✅ Keep your device secure
- ✅ Use trusted networks for upload/download
- ✅ Don't use on public/shared computers
- ⚠️ Credentials stored locally - protect your device

---

## 🛠️ Security Checklist for Developers

If you're forking or deploying BetterSave:

- [ ] Create your own Firebase project
- [ ] Generate new API keys (don't use defaults)
- [ ] Deploy Firebase Security Rules
- [ ] Test all authentication flows
- [ ] Verify user data isolation
- [ ] Test with malicious payloads
- [ ] Monitor Firebase usage and costs
- [ ] Set up billing alerts
- [ ] Review code for hardcoded credentials
- [ ] Update API_KEY in FirebaseAuth.cpp

---

## 🐛 Reporting Security Vulnerabilities

### What to Report

We take all security issues seriously. Please report:

- Authentication bypass vulnerabilities
- Unauthorized data access
- Data integrity issues
- Token theft or session hijacking
- Denial of service vulnerabilities
- Code execution vulnerabilities
- Any other security concerns

### How to Report

**DO NOT** open public GitHub issues for security vulnerabilities!

Instead, please:

1. **Email**: Send details to the project maintainer (sidastuff)
   - Include detailed description
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if any)

2. **Private disclosure**: Use GitHub Security Advisories (if available)

3. **Expected response**: We aim to respond within 48-72 hours

### What to Include

- Detailed description of the vulnerability
- Steps to reproduce
- Affected versions
- Potential impact/severity
- Any proof-of-concept code
- Suggested remediation (if applicable)

---

## 🎯 Security Roadmap

Potential future security enhancements:

### High Priority
- [ ] Password reset functionality
- [ ] Two-factor authentication (2FA)
- [ ] Email verification on signup
- [ ] Server-side rate limiting (Cloud Functions)

### Medium Priority
- [ ] Save file encryption at rest
- [ ] Audit logs for all operations
- [ ] Account activity monitoring
- [ ] IP-based rate limiting

### Low Priority
- [ ] OAuth providers (Google, Discord)
- [ ] Account deletion with data purge
- [ ] GDPR compliance features
- [ ] Data export functionality

---

## 📚 Security Resources

### Firebase Security
- [Firebase Security Rules Guide](https://firebase.google.com/docs/database/security)
- [Firebase Auth Best Practices](https://firebase.google.com/docs/auth/security)
- [Securing Firebase Realtime Database](https://firebase.google.com/docs/database/security/securing-data)

### General Security
- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [API Security Best Practices](https://owasp.org/www-project-api-security/)

---

## 📋 Security Audit History

### v1.2.0
- ✅ Reviewed authentication flow
- ✅ Created comprehensive Firebase rules
- ✅ Added security documentation
- ✅ No vulnerabilities found

### v1.0.0
- ✅ Initial security review
- ✅ Basic Firebase rules implemented
- ✅ Token-based auth working

---

## ⚖️ Responsible Disclosure

We believe in responsible disclosure:

1. **Reporter**: Privately disclose vulnerability
2. **Team**: Acknowledge receipt within 48-72 hours
3. **Team**: Investigate and develop fix
4. **Team**: Release patched version
5. **Reporter**: Publicly disclose after patch (if desired)
6. **Team**: Credit reporter in changelog (with permission)

---

## 🙏 Security Credits

We appreciate security researchers who help keep BetterSave secure.

**Contributors:**
- *None yet - be the first!*

---

## 📄 License & Legal

Security vulnerabilities should be reported privately. Public disclosure without giving us time to patch may put users at risk.

By using BetterSave, you agree to:
- Not attempt to compromise other users' data
- Not abuse the service (spam, DOS attacks, etc.)
- Report security issues responsibly
- Use the mod in good faith

---

**Last Updated**: v1.2.0
**Maintained by**: sidastuff

For security questions, contact the maintainer through appropriate private channels.

