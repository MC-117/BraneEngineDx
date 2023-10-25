#include "Serialization.h"

StaticVar<map<string, Serialization*>> SerializationManager::serializationList;

Serialization* SerializationManager::getSerialization(const string& type)
{
	auto iter = serializationList->find(type);
	if (iter == serializationList->end())
		return NULL;
	return iter->second;
}

void SerializationInfo::clear()
{
	type.clear();
	arrayType.clear();
	path.clear();
	name.clear();
	serialization = NULL;
	stringFeild.clear();
	numFeild.clear();
	subfeilds.clear();
}

bool SerializationInfo::isArrayOf(const string & type)
{
	if (type != "Array")
		return false;
	return arrayType == type;
}

bool SerializationInfo::consistKey(const string & name)
{
	auto itern = numFeild.find(name);
	if (itern != numFeild.end())
		return true;
	auto iters = stringFeild.find(name);
	if (iters != stringFeild.end())
		return true;
	auto itersf = subfeilds.find(name);
	if (itersf != subfeilds.end())
		return true;
	return false;
}

bool SerializationInfo::add(const string & name, Decimal value)
{
	if (consistKey(name))
		return false;
	numFeild.insert(pair<string, size_t>(name, numList.size()));
	numList.push_back(value);
	return true;
}

bool SerializationInfo::add(const string & name, const string & value)
{
	if (consistKey(name))
		return false;
	stringFeild.insert(pair<string, size_t>(name, stringList.size()));
	stringList.push_back(value);
	return true;
}

SerializationInfo * SerializationInfo::add(const string & name)
{
	if (consistKey(name))
		return NULL;
	subfeilds.insert(pair<string, size_t>(name, sublists.size()));
	SerializationInfo& info = sublists.emplace_back();
	info.name = name;
	info.path = path;
	return &info;
}

void SerializationInfo::push(Decimal value)
{
	numFeild.insert(pair<string, size_t>(to_string(numFeild.size()), numFeild.size()));
	numList.push_back(value);
}

void SerializationInfo::push(const string & value)
{
	stringFeild.insert(pair<string, size_t>(to_string(stringFeild.size()), stringFeild.size()));
	stringList.push_back(value);
}

SerializationInfo * SerializationInfo::push()
{
	subfeilds.insert(pair<string, size_t>(to_string(subfeilds.size()), subfeilds.size()));
	SerializationInfo& info = sublists.emplace_back();
	info.path = path;
	info.type = arrayType;
	info.name = to_string(subfeilds.size() - 1);
	return &info;
}

void SerializationInfo::set(const string & name, const string & value)
{
	auto iter = stringFeild.find(name);
	if (iter == stringFeild.end()) {
		stringFeild.insert(pair<string, size_t>(name, stringList.size()));
		stringList.push_back(value);
	}
	else
		stringList[iter->second] = value;
}

void SerializationInfo::set(const string & name, Decimal value)
{
	auto iter = numFeild.find(name);
	if (iter == numFeild.end()) {
		numFeild.insert(pair<string, size_t>(name, numList.size()));
		numList.push_back(value);
	}
	else
		numList[iter->second] = value;
}

void SerializationInfo::set(const SerializationInfo& value)
{
	auto iter = subfeilds.find(value.name);
	SerializationInfo* info = NULL;
	if (iter == subfeilds.end()) {
		subfeilds.insert(pair<string, size_t>(value.name, stringList.size()));
		info = &sublists.emplace_back(value);
	}
	else
		info = &(sublists[iter->second] = value);
	info->path = path;
}

void SerializationInfo::set(const string & name, Serializable & value)
{
	auto iter = subfeilds.find(name);
	SerializationInfo* info;
	if (iter == subfeilds.end()) {
		subfeilds.insert(pair<string, size_t>(name, sublists.size()));
		info = &sublists.emplace_back();
	}
	else {
		info = &sublists[iter->second];
		info->clear();
	}
	info->path = path;
	info->type = value.getSerialization().type;
	info->name = name;
	if (!value.serialize(*info))
		throw runtime_error("Serializable serialize failed");
}

const SerializationInfo * SerializationInfo::get(const Path & path) const
{
	if (path.empty())
		return NULL;
	if (path.size() == 1) {
		auto iter = subfeilds.find(path[0]);
		if (iter == subfeilds.end())
			return NULL;
		return &sublists[iter->second];
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return NULL;
		return sublists[_iter->second].get(path.popTop());
	}
}

SerializationInfo * SerializationInfo::get(const Path & path)
{
	if (path.empty())
		return NULL;
	if (path.size() == 1) {
		auto iter = subfeilds.find(path[0]);
		if (iter == subfeilds.end())
			return NULL;
		return &sublists[iter->second];
	}
	else {
		auto _iter = subfeilds.find(path[0]);
		if (_iter == subfeilds.end())
			return NULL;
		return sublists[_iter->second].get(path.popTop());
	}
}

const SerializationInfo * SerializationInfo::get(const size_t i) const
{
	if (i < sublists.size()) {
		return &sublists[i];
	}
	return NULL;
}

SerializationInfo * SerializationInfo::get(const size_t i)
{
	if (i < sublists.size()) {
		return &sublists[i];
	}
	return NULL;
}

Serializable* Serializable::instantiate(const SerializationInfo& from)
{
	return NULL;
}

void Serializable::serializeInit(const Serializable* serializable, SerializationInfo& to)
{
	to.type = serializable->getSerialization().type;
}

map<filesystem::path, SerializationInfo*> Serialization::serializationInfoByPath;

Serialization* Serialization::getBaseSerialization()
{
	if (baseType.empty())
		return NULL;
	if (baseSerialization == NULL) {
		baseSerialization = SerializationManager::getSerialization(baseType);
	}
	return baseSerialization;
}

Serialization* Serialization::getBaseSerialization() const
{
	if (baseType.empty())
		return NULL;
	if (baseSerialization == NULL) {
		return SerializationManager::getSerialization(baseType);
	}
	return baseSerialization;
	//return baseSerialization == &serialization ? NULL : baseSerialization;
}

bool Serialization::isChildOf(const Serialization& serialization)
{
	if (&serialization == NULL)
		return false;
	Serialization* base = this;
	while (base != NULL) {
		base = base->getBaseSerialization();
		if (base == &serialization)
			return true;
	}
	return false;
}

bool Serialization::isChildOf(const Serialization& serialization) const
{
	if (&serialization == NULL)
		return false;
	const Serialization* base = this;
	while (base != NULL) {
		base = base->getBaseSerialization();
		if (base == &serialization)
			return true;
	}
	return false;
}

int Serialization::getChildren(vector<Serialization*>& children) const
{
	int count = 0;
	for (auto b = SerializationManager::serializationList->begin(),
		e = SerializationManager::serializationList->end(); b != e; b++) {
		if (b->second->isChildOf(*this)) {
			children.push_back(b->second);
			count++;
		}
	}
	return count;
}

int Serialization::getAttributeCount() const
{
	return attributes.size();
}
Attribute* Serialization::getAttribute(int index) const
{
	if (index >= attributes.size())
		return NULL;
	return attributes[index];
}

Attribute* Serialization::getAttribute(const string& name) const
{
	auto iter = attributeNameMap.find(name);
	if (iter == attributeNameMap.end())
		return NULL;
	return iter->second;
}

bool Serialization::addInfo(SerializationInfo & info, bool overwrite)
{
	if (info.name.empty())
		return false;
	auto iter = serializationInfoByName.find(info.name);
	if (iter != serializationInfoByName.end()) {
		if (overwrite) {
			delete iter->second;
			iter->second = &info;
		}
		else
			return false;
	}
	else
		serializationInfoByName.insert(pair<string, SerializationInfo*>(info.name, &info));
	if (!info.path.empty()) {
		auto _iter = serializationInfoByPath.find(info.path);
		if (_iter == serializationInfoByPath.end())
			serializationInfoByPath.insert(make_pair(info.path, &info));
		else if (overwrite) {
			_iter->second = &info;
		}
	}
	info.serialization = this;
	return true;
}

SerializationInfo * Serialization::getInfoByName(const string & name)
{
	auto iter = serializationInfoByName.find(name);
	if (iter == serializationInfoByName.end())
		return NULL;
	return iter->second;
}

SerializationInfo * Serialization::getInfoByPath(const string & path)
{
	auto iter = serializationInfoByPath.find(path);
	if (iter == serializationInfoByPath.end())
		return NULL;
	return iter->second;
}

Serializable * Serialization::deserializeByName(const string & name)
{
	SerializationInfo* info = getInfoByName(name);
	if (info == NULL)
		return NULL;
	return deserialize(*info);
}

Serializable * Serialization::deserializeByPath(const string & name)
{
	SerializationInfo* info = getInfoByPath(name);
	if (info == NULL)
		return NULL;
	return info->serialization->deserialize(*info);
}

bool Serialization::serialize(Serializable & object, const string & path, const string & name)
{
	if (path.empty())
		return false;
	SerializationInfo* info = new SerializationInfo();
	if (!object.serialize(*info)) {
		delete info;
		return false;
	}
	addInfo(*info, true);
	return true;
}

Serializable * Serialization::clone(Serializable & object)
{
	Serialization& serialization = object.getSerialization();
	SerializationInfo info;
	object.serialize(info);
	Serializable* re = serialization.instantiate(info);
	if (re != NULL)
		re->deserialize(info);
	return re;
}

SerializationScope* SerializationScope::currentScope = NULL;

Serialization Serialization::serialization("Serialization", "");

Serialization::~Serialization()
{
	for each (auto attr in attributes)
	{
		delete attr;
	}
	attributes.clear();
}

void Serialization::init()
{
	SerializationScope* scope = SerializationScope::getScope();
	if (scope)
		scope->serializationConstuction(this);
}

void Serialization::addAttribute(const vector<Attribute*>& list)
{
	for each (auto attr in list)
	{
		Attribute* raw = getAttribute(attr->name);
		if (raw) {
			raw->resolve(attr);
			delete attr;
		}
		else {
			attributeNameMap[attr->name] = attr;
			attributes.push_back(attr);
		}
	}
}

Serialization::Serialization(const string& type, const string& baseType) : type(type), baseType(baseType)
{
	auto iter = SerializationManager::serializationList->find(type);
	if (iter == SerializationManager::serializationList->end())
		SerializationManager::serializationList->insert(pair<string, Serialization*>(type, this));
	init();
}

Serialization::Serialization(const char* type, const char* baseType) : type(type), baseType(baseType)
{
	auto iter = SerializationManager::serializationList->find(type);
	if (iter == SerializationManager::serializationList->end())
		SerializationManager::serializationList->insert(pair<string, Serialization*>(type, this));
	init();
}

SerializationInfoParser::SerializationInfoParser(istream & is, const string& path)
	: stream(is), path(path)
{
}

char SerializationInfoParser::getNextChar()
{
	char c;
	if (!stream.get(c))
		return '\0';
	return c;
}

void SerializationInfoParser::ungetNextChar()
{
	stream.unget();
}

SerializationInfoParser::TokenType SerializationInfoParser::getToken()
{
	TokenType currentToken;
	StateType state = START;
	bool save;
	int isfloat = 0;
	while (state != DONE)
	{
		char c = getNextChar();
		save = true;
		switch (state)
		{
		case START:
			if (isdigit(c)) {
				state = INNUM;
				isfloat = false;
			}
			else if (isalpha(c) || c == '_')
				state = INNAME;
			else if (isspace(c) && state != INSTRING)
				save = false;
			else
			{
				state = DONE;
				switch (c)
				{
				case '\0':
					save = false;
					currentToken = ENDFILE;
					break;
				case '{':
					currentToken = LBR;
					break;
				case '}':
					currentToken = RBR;
					break;
				case '[':
					currentToken = LSBR;
					break;
				case ']':
					currentToken = RSBR;
					break;
				case ':':
					currentToken = COLON;
					break;
				case ',':
					currentToken = COMMA;
					break;
				case '"':
					save = false;
					state = INSTRING;
					break;
				case '-':
					state = INNUM;
					isfloat = false;
					break;
				case '+':
					state = INNUM;
					isfloat = false;
					break;
				case '.':
					state = INNUM;
					isfloat = true;
					break;
				default:
					currentToken = ERR;
					break;
				}
			}
			break;
		case INNUM:
			if (!isdigit(c))
			{
				if (c == '.' && isfloat == 0)
					isfloat = 1;
				else if (c == 'e' && isfloat == 0)
					isfloat = 2;
				else if (c == 'e' && isfloat == 1)
					isfloat = 2;
				else if (c == '+' && isfloat == 2)
					isfloat = 3;
				else if (c == '-' && isfloat == 2)
					isfloat = 3;
				else {
					ungetNextChar();
					save = false;
					state = DONE;
					currentToken = isfloat ? FLOAT : INT;
				}
			}
			break;
		case INNAME:
			if (!isalpha(c) && !isdigit(c) && c != '_' && c != '-')
			{
				ungetNextChar();
				save = false;
				state = DONE;
				currentToken = NAME;
			}
			break;
		case INSTRING:
			if (c == '"')
			{
				save = false;
				state = DONE;
				currentToken = STRING;
			}
			break;
		case DONE:
		default:
			cout << "Scanner Bug: state=" << state << endl;
			state = DONE;
			currentToken = ERR;
			break;
		}
		if (save)
			tokenString += c;
		if (state == DONE)
		{
			if (currentToken == ERR)
				errorString = tokenString;
		}
	}
	//cout << "Token: " << currentToken << " " << tokenString << endl;
	return currentToken;
}

void SerializationInfoParser::findError(string msg, bool scannerError)
{
	if (parseError.empty())
	{
		if (scannerError)
			parseError = "scanner error, " + msg + " is a problem at pos " + to_string(stream.tellg());
		else
			parseError = "syntax error, " + msg + " is a problem at pos " + to_string(stream.tellg());
	}
	Error = true;
}

void SerializationInfoParser::match(TokenType expected)
{
	if (token == expected)
	{
		backupTokenString = tokenString;
		tokenString.clear();
		backupToken = token;
		token = getToken();
	}
	else
	{
		if (token == ENDFILE)
			findError(backupTokenString);
		else
			findError(tokenString);
	}
}

bool SerializationInfoParser::object_sequence()
{
	while (true) {
		string name, type;
		if (token == NAME) {
			match(NAME);
			if (token == COLON) {
				name = backupTokenString;
				match(COLON);
				if (token == NAME) {
					type = tokenString;
					match(NAME);
				}
				else {
					findError("Unexpected token: " + tokenString);
					return false;
				}
			}
			else
				type = backupTokenString;
		}

		if (token == LBR) {
			SerializationInfo& info = infos.emplace_back();
			info.name = name;
			info.path = path;
			info.type = type;
			if (!object(info)) {
				findError("Unexpected token: " + tokenString);
				return false;
			}
		}
		else if (token == LSBR) {
			bool isObjectArray = backupToken == NAME;
			match(LSBR);
			SerializationInfo& info = infos.emplace_back();
			info.name = name;
			info.path = path;
			bool arraySuccess = true;
			if (isObjectArray)
				arraySuccess = object_array(info);
			else
				arraySuccess = array(info);
			if (!arraySuccess) {
				return false;
			}
		}
		else if (token == COMMA) {
			continue;
		}
		else if (token == ENDFILE)
			break;
		else {
			findError("Unexpected token: " + tokenString);
			return false;
		}
	}
	return true;
}

bool SerializationInfoParser::object(SerializationInfo & info)
{
	if (token == LBR) {
		match(LBR);
		while (true) {
			if (token == RBR) {
				match(RBR);
				break;
			}
			string name = tokenString;
			if (token == NAME || token == STRING)
				match(token);
			else
				match(NAME);
			match(COLON);
			if (!factor(info, name))
				return false;
			if (token == COMMA) {
				match(COMMA);
				if (token == RBR) {
					match(RBR);
					break;
				}
			}
		}
		info.serialization = SerializationManager::getSerialization(info.type);
	}
	else if (token == LSBR) {
		match(LSBR);
		info.arrayType = info.type;
		info.type = "Array";
		object_array(info);
	}
	else {
		findError("Unexpected token: " + tokenString);
		return false;
	}
	return true;
}

bool SerializationInfoParser::array(SerializationInfo & info)
{
	TokenType _token = token;
	info.type = "Array";
	int size = 0;
	while (true) {
		if (token == RSBR) {
			match(RSBR);
			break;
		}
		if (!factor(info, to_string(size)))
			return false;
		if (backupToken != _token) {
			findError("Do not allow different type in an array");
			return false;
		}
		if (token == COMMA) {
			match(COMMA);
			if (token == RSBR) {
				match(RSBR);
				break;
			}
		}
		else {
			findError("Unexpected token: " + tokenString);
			return false;
		}
		size++;
	}
	info.arrayType = (_token == INT | _token == FLOAT) ? "Number" : "String";
	return true;
}

bool SerializationInfoParser::object_array(SerializationInfo & info)
{
	int size = 0;
	while (true) {
		if (token == RSBR) {
			match(RSBR);
			break;
		}
		SerializationInfo* pinfo = info.push();
		pinfo->type = info.arrayType;
		if (!object(*pinfo))
			return false;
		if (token == COMMA) {
			match(COMMA);
			if (token == RSBR) {
				match(RSBR);
				break;
			}
		}
		else {
			findError("Unexpected token: " + tokenString);
			return false;
		}
		size++;
	}
	return true;
}

bool SerializationInfoParser::factor(SerializationInfo & info, const string& name)
{
	SerializationInfo* pinfo = NULL;
	switch (token)
	{
	case INT:
		if (!info.add(name, atoll(tokenString.c_str()))) {
			findError("Same key name: " + name);
			return false;
		}
		match(INT);
		break;
	case FLOAT:
		if (!info.add(name, atof(tokenString.c_str()))) {
			findError("Same key name: " + name);
			return false;
		}
		match(FLOAT);
		break;
	case STRING:
		if (!info.add(name, tokenString)) {
			findError("Same key name: " + name);
			return false;
		}
		match(STRING);
		break;
	case NAME:
		pinfo = info.add(name);
		if (pinfo == NULL) {
			findError("Same key name: " + name);
			return false;
		}
		pinfo->type = tokenString;
		match(NAME);
		return object(*pinfo);
	case LSBR:
		pinfo = info.add(name);
		if (pinfo == NULL) {
			findError("Same key name: " + name);
			return false;
		}
		match(LSBR);
		return array(*pinfo);
	default:
		findError("Unexpected token: " + tokenString);
		return false;
	}
	return true;
}

bool SerializationInfoParser::parse()
{
	if (stream.fail())
		return false;
	parseError.clear();
	token = getToken();
	if (!object_sequence())
		return false;
	if (token != ENDFILE)
		findError(tokenString);
	return !infos.empty();
}

SerializationInfoWriter::SerializationInfoWriter(ostream & os) : stream(os)
{
}

string SerializationInfoWriter::checkName(const string & name)
{
	if (name.empty())
		throw runtime_error("Empty name");
	if (isdigit(name[0]))
		return '\"' + name + '\"';
	for (int i = 0; i < name.size(); i++) {
		if (!isalpha(name[i]) && name[i] != '_' && !isdigit(name[i]))
			return '\"' + name + '\"';
	}
	return name;
}

void SerializationInfoWriter::write(const SerializationInfo & info, bool showType)
{
	if (!info.name.empty())
		stream << checkName(info.name) << ": ";
	internalWrite(info, showType);
}

void SerializationInfoWriter::internalWrite(const SerializationInfo& info, bool showType)
{
	if (info.type == "Array") {
		if (info.arrayType == "Number") {
			stream << "[ ";
			for (auto b = info.numList.begin(), e = info.numList.end(); b != e; b++) {
				stream << b->toString() << ", ";
			}
			stream << " ]";
		}
		else if (info.arrayType == "String") {
			stream << '[';
			for (auto b = info.stringList.begin(), e = info.stringList.end(); b != e; b++) {
				stream << '\"' << *b << '\"' << ", ";
			}
			stream << " ]";
		}
		else {
			stream << info.arrayType << " [ ";
			for (auto b = info.sublists.begin(), e = info.sublists.end(); b != e; b++) {
				internalWrite(*b, false);
				stream << ", ";
			}
			stream << " ]";
		}
	}
	else {
		if (showType)
			stream << (info.type.empty() ? "None" : info.type) << ' ';
		stream << "{ ";
		map<int, string> sortedMap;
		for (auto b = info.numFeild.begin(), e = info.numFeild.end(); b != e; b++) {
			sortedMap.insert({ b->second, b->first });
		}
		for (auto b = sortedMap.begin(), e = sortedMap.end(); b != e; b++) {
			stream << checkName(b->second) << ": " << info.numList[b->first].toString() << ", ";
		}
		sortedMap.clear();
		for (auto b = info.stringFeild.begin(), e = info.stringFeild.end(); b != e; b++) {
			sortedMap.insert({ b->second, b->first });
		}
		for (auto b = sortedMap.begin(), e = sortedMap.end(); b != e; b++) {
			stream << checkName(b->second) << ": \"" << info.stringList[b->first] << "\", ";
		}
		sortedMap.clear();
		for (auto b = info.subfeilds.begin(), e = info.subfeilds.end(); b != e; b++) {
			sortedMap.insert({ b->second, b->first });
		}
		for (auto b = sortedMap.begin(), e = sortedMap.end(); b != e; b++) {
			stream << checkName(b->second) << ": ";
			internalWrite(info.sublists[b->first]);
			stream << ", ";
		}
		stream << " }";
	}
}

SerializeInstance(SVector2f);

SVector2f::SVector2f(float x, float y) : x(x), y(y)
{
}

SVector2f::SVector2f(const Vector2f& vec) : x(vec[0]), y(vec[1])
{
}

bool SVector2f::deserialize(const SerializationInfo & from)
{
	if (!from.get<float>(Path("x"), x))
		return false;
	if (!from.get<float>(Path("y"), y))
		return false;
	return true;
}

bool SVector2f::serialize(SerializationInfo & to)
{
	serializeInit(this, to);
	to.set("x", x);
	to.set("y", y);
	return true;
}

SVector2f & SVector2f::operator=(const Vector2f& v)
{
	x = v[0];
	y = v[1];
	return *this;
}

SVector2f::operator Vector2f()
{
	return Vector2f(x, y);
}

SVector2f::operator Vector2f() const
{
	return Vector2f(x, y);
}

SerializeInstance(SVector3f);

SVector3f::SVector3f(float x, float y, float z)
	: x(x), y(y), z(z)
{
}

SVector3f::SVector3f(const Vector3f& vec)
	: x(vec[0]), y(vec[1]), z(vec[2])
{
}

bool SVector3f::deserialize(const SerializationInfo & from)
{
	if (!from.get<float>(Path("x"), x))
		return false;
	if (!from.get<float>(Path("y"), y))
		return false;
	if (!from.get<float>(Path("z"), z))
		return false;
	return true;
}

bool SVector3f::serialize(SerializationInfo & to)
{
	serializeInit(this, to);
	to.set("x", x);
	to.set("y", y);
	to.set("z", z);
	return true;
}

SVector3f & SVector3f::operator=(const Vector3f& v)
{
	x = v[0];
	y = v[1];
	z = v[2];
	return *this;
}

SVector3f::operator Vector3f()
{
	return Vector3f(x, y, z);
}

SVector3f::operator Vector3f() const
{
	return Vector3f(x, y, z);
}

SerializeInstance(SQuaternionf);

SQuaternionf::SQuaternionf(float x, float y, float z, float w)
	: x(x), y(y), z(z), w(w)
{
}

SQuaternionf::SQuaternionf(const Quaternionf & quat)
	: x(quat.x()), y(quat.y()), z(quat.z()), w(quat.w())
{
}

bool SQuaternionf::deserialize(const SerializationInfo & from)
{
	if (!from.get<float>(Path("x"), x))
		return false;
	if (!from.get<float>(Path("y"), y))
		return false;
	if (!from.get<float>(Path("z"), z))
		return false;
	if (!from.get<float>(Path("w"), w))
		return false;
	return true;
}

bool SQuaternionf::serialize(SerializationInfo & to)
{
	serializeInit(this, to);
	to.set("x", x);
	to.set("y", y);
	to.set("z", z);
	to.set("w", w);
	return true;
}

SQuaternionf & SQuaternionf::operator=(const Quaternionf & v)
{
	x = v.x();
	y = v.y();
	z = v.z();
	w = v.w();
	return *this;
}

SQuaternionf::operator Quaternionf()
{
	return Quaternionf(w, x, y, z);
}

SQuaternionf::operator Quaternionf() const
{
	return Quaternionf(w, x, y, z);
}

SerializeInstance(SColor);

SColor::SColor(float r, float g, float b, float a)
	: r(r), g(g), b(b), a(a)
{
}

SColor::SColor(const Color& color)
	: r(color.r), g(color.g), b(color.b), a(color.a)
{
}

bool SColor::deserialize(const SerializationInfo& from)
{
	if (!from.get<float>(Path("r"), r))
		return false;
	if (!from.get<float>(Path("g"), g))
		return false;
	if (!from.get<float>(Path("b"), b))
		return false;
	if (!from.get<float>(Path("a"), a))
		return false;
	return true;
}

bool SColor::serialize(SerializationInfo& to)
{
	serializeInit(this, to);
	to.set("r", r);
	to.set("g", g);
	to.set("b", b);
	to.set("a", a);
	return true;
}

SColor& SColor::operator=(const Color& c)
{
	r = c.r;
	g = c.g;
	b = c.b;
	a = c.a;
	return *this;
}

SColor::operator Color()
{
	return Color(r, g, b, a);
}

SColor::operator Color() const
{
	return Color(r, g, b, a);
}
