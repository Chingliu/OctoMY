/* This file is distributed under the license as specified in the LICENSE file
 * that accompanied it. If there was no LICENSE file accompanying this source,
 * it is not to be modified, redistributed or used without prior written
 * permission from original author and owner "Lennart Rolland".
 */

#ifndef ASYNCSTORE_HPP
#define ASYNCSTORE_HPP

#include "basic/AtomicBoolean.hpp"

#include "GenerateRunnable.hpp"

#include <QObject>
#include <QFile>

class AsyncStore : public QObject
{
	Q_OBJECT
protected:

	AtomicBoolean mReady;
	AtomicBoolean mError;
	AtomicBoolean mInProgress;

	const QString mFilename;


public:
	explicit AsyncStore(QString="", QObject *parent = nullptr);


protected:

	void bootstrapWorker();

	virtual void bootstrapWorkerImpl()=0;
public:
	virtual void load()=0;
	virtual void save()=0;

public:


	// Make if needed, load if otherwise
	void bootstrap(bool loadOnly=false, bool runInBackground=true);

	bool isReady();
	bool isInProgress();
	bool hasError();
	bool fileExists() const;

	friend class GenerateRunnable<AsyncStore>;

signals:

	void storeReady(bool ok);
};

#endif // ASYNCSTORE_HPP
