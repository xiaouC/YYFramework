#include "Map/TLSeamlessMap.h"
#include "Map/TLMapBlock.h"
#include "MC/AssetsManager.h"
#include "map.pb.h"

const float g_fLoadNewBlockFlag = 0.2f;
TLSeamlessMap::TLSeamlessMap( const std::string& strSeamlessMapFile, float x, float y )
: m_strSeamlessMapFile(strSeamlessMapFile)
, m_fCurX(x)
, m_fCurY(y)
, m_nBlockRow(0)
, m_nBlockCol(0)
, m_nGridWidth(0)
, m_nGridHeight(0)
, m_nBlockWidth(0)
, m_nBlockHeight(0)
{
    for( int i=BLOCK_INDEX_CENTER; i < BLOCK_INDEX_MAX; ++i )
    {
        m_kBlocks[i].pMapBlock = NULL;
        m_kBlocks[i].x = 0.0f;
        m_kBlocks[i].y = 0.0f;

        m_kOldBlocks[i].pMapBlock = NULL;
        m_kOldBlocks[i].x = 0.0f;
        m_kOldBlocks[i].y = 0.0f;
    }
}

TLSeamlessMap::~TLSeamlessMap()
{
    std::list<BlockInfo*>::iterator iter = m_listAllBlocks.begin();
    std::list<BlockInfo*>::iterator iter_end = m_listAllBlocks.end();
    for( ; iter != iter_end; ++iter )
    {
        BlockInfo* pBlockInfo = (*iter);
        delete pBlockInfo;
    }
    m_listAllBlocks.clear();
}

TLSeamlessMap* TLSeamlessMap::create( const std::string& strSeamlessMapFile, float x, float y )
{
    TLSeamlessMap * pRet = new TLSeamlessMap( strSeamlessMapFile, x, y );
    if( pRet && pRet->init() )
    {
        pRet->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(pRet);
    }
    return pRet;
}

bool TLSeamlessMap::newSeamlessMap( const std::string& strSeamlessMapFile, int nBlockRow, int nBlockCol, int nGridWidth, int nGridHeight )
{
    std::string strFirstBlockFileName = strSeamlessMapFile + ".mb";
    if( !TLMapBlock::newMapBlock( strFirstBlockFileName, nBlockRow, nBlockCol, nGridWidth, nGridHeight ) )
        return false;

    std::string strSMFileName = strSeamlessMapFile + ".sm";

    FILE* fp = fopen( strSMFileName.c_str(), "wb" );
    if( fp == NULL )
        return false;

    framework::SeamlessMap smData;
    smData.set_blockrow( nBlockRow );
    smData.set_blockcol( nBlockCol );
    smData.set_gridwidth( nGridWidth );
    smData.set_gridheight( nGridHeight );

    framework::BlockInfo* bi = smData.add_blocks();
    bi->set_file( strFirstBlockFileName );
    bi->set_x( 0 );
    bi->set_y( 0 );

    std::string strBuffer;
    smData.SerializeToString( &strBuffer );

    fwrite( strBuffer.c_str(), strBuffer.length(), 1, fp );

    // 
    fclose( fp );

    return true;
}

bool TLSeamlessMap::init()
{
    if( !CCNode::init() )
        return false;

    unsigned long iSize = 0;
	unsigned char* pBuffer = AssetsManager::sharedAssetsManager()->getFileContent( m_strSeamlessMapFile.c_str(), "rb", &iSize );
    if( pBuffer == NULL )
        return false;

    framework::SeamlessMap smData;
	smData.ParseFromArray( (void*)pBuffer, iSize );

	delete[] pBuffer;

    m_nBlockRow = smData.blockrow();
    m_nBlockCol = smData.blockcol();
    m_nGridWidth = smData.gridwidth();
    m_nGridHeight = smData.gridheight();

    m_nBlockWidth = m_nBlockCol * m_nGridWidth;
    m_nBlockHeight = m_nBlockRow * m_nGridHeight;

    for( int i=0; i < smData.blocks_size(); ++i )
    {
        const framework::BlockInfo& bi = smData.blocks( i );

        BlockInfo* pBlockInfo = new BlockInfo;
        pBlockInfo->strBlockFileName = bi.file();
        pBlockInfo->x = bi.x();
        pBlockInfo->y = bi.y();
        m_listAllBlocks.push_back( pBlockInfo );
    }

    updateBlock();

    return true;
}

TLMapBlock* TLSeamlessMap::loadBlock( const std::string& strFileName )
{
    return TLMapBlock::create( strFileName );
}

void TLSeamlessMap::updateBlock()
{
    // 当前块的中心坐标
    float fCurCenterX = m_fCurX;
    float fCurCenterY = m_fCurY;
    correctCoordinate( fCurCenterX, fCurCenterY );

    // 先备份
    for( int i=BLOCK_INDEX_CENTER; i < BLOCK_INDEX_MAX; ++i )
    {
        m_kOldBlocks[i] = m_kBlocks[i];

        m_kBlocks[i].pMapBlock = NULL;
        m_kBlocks[i].strBlockFileName.clear();
        m_kBlocks[i].x = 0.0f;
        m_kBlocks[i].y = 0.0f;
    }

    // 当前位置的
    BlockInfo* pCenterBlockInfo = getBlockInfo( m_fCurX, m_fCurY );
    updateBlock( BLOCK_INDEX_CENTER, pCenterBlockInfo );

    float fLeftFlag = ( m_fCurX - ( fCurCenterX - m_nBlockWidth * 0.5f ) ) / m_nBlockWidth;
    float fRightFlag = ( ( fCurCenterX + m_nBlockWidth * 0.5f ) - m_fCurX ) / m_nBlockWidth;
    float fTopFlag = ( ( fCurCenterY + m_nBlockHeight * 0.5f ) - m_fCurY ) / m_nBlockHeight;
    float fBottomFlag = ( m_fCurY - ( fCurCenterY - m_nBlockHeight * 0.5f ) ) / m_nBlockHeight;

    // 左
    if( fLeftFlag <= g_fLoadNewBlockFlag )
    {
        BlockInfo* pBlockInfo = getBlockInfo( m_fCurX - m_nBlockWidth, m_fCurY );
        updateBlock( BLOCK_INDEX_LEFT, pBlockInfo );
    }

    // 右
    if( fRightFlag <= g_fLoadNewBlockFlag )
    {
        BlockInfo* pBlockInfo = getBlockInfo( m_fCurX - m_nBlockWidth, m_fCurY );
        updateBlock( BLOCK_INDEX_RIGHT, pBlockInfo );
    }

    // 上
    if( fTopFlag <= g_fLoadNewBlockFlag )
    {
        BlockInfo* pBlockInfo = getBlockInfo( m_fCurX, m_fCurY + m_nBlockHeight );
        updateBlock( BLOCK_INDEX_TOP, pBlockInfo );
    }

    // 下
    if( fBottomFlag <= g_fLoadNewBlockFlag )
    {
        BlockInfo* pBlockInfo = getBlockInfo( m_fCurX, m_fCurY - m_nBlockHeight );
        updateBlock( BLOCK_INDEX_BOTTOM, pBlockInfo );
    }

    // 左上
    if( fLeftFlag <= g_fLoadNewBlockFlag && fTopFlag <= g_fLoadNewBlockFlag )
    {
        BlockInfo* pBlockInfo = getBlockInfo( m_fCurX - m_nBlockWidth, m_fCurY + m_nBlockHeight );
        updateBlock( BLOCK_INDEX_LEFT_TOP, pBlockInfo );
    }

    // 左下
    if( fLeftFlag <= g_fLoadNewBlockFlag && fBottomFlag <= g_fLoadNewBlockFlag )
    {
        BlockInfo* pBlockInfo = getBlockInfo( m_fCurX - m_nBlockWidth, m_fCurY - m_nBlockHeight );
        updateBlock( BLOCK_INDEX_LEFT_BOTTOM, pBlockInfo );
    }

    // 右上
    if( fRightFlag <= g_fLoadNewBlockFlag && fTopFlag <= g_fLoadNewBlockFlag )
    {
        BlockInfo* pBlockInfo = getBlockInfo( m_fCurX + m_nBlockWidth, m_fCurY + m_nBlockHeight );
        updateBlock( BLOCK_INDEX_RIGHT_TOP, pBlockInfo );
    }

    // 右下
    if( fRightFlag <= g_fLoadNewBlockFlag && fBottomFlag <= g_fLoadNewBlockFlag )
    {
        BlockInfo* pBlockInfo = getBlockInfo( m_fCurX + m_nBlockWidth, m_fCurY - m_nBlockHeight );
        updateBlock( BLOCK_INDEX_RIGHT_BOTTOM, pBlockInfo );
    }

    // 把备份删掉
    for( int i=BLOCK_INDEX_CENTER; i < BLOCK_INDEX_MAX; ++i )
    {
        if( m_kOldBlocks[i].pMapBlock != NULL )
        {
            m_kOldBlocks[i].pMapBlock->removeFromParentAndCleanup( true );

            m_kOldBlocks[i].pMapBlock = NULL;
            m_kOldBlocks[i].strBlockFileName.clear();
            m_kOldBlocks[i].x = 0.0f;
            m_kOldBlocks[i].y = 0.0f;
        }
    }
}

void TLSeamlessMap::updateBlock( int nIndex, BlockInfo* pBlockInfo )
{
    if( pBlockInfo == NULL )
        return;

    // 在备份里面查找
    for( int i=BLOCK_INDEX_CENTER; i < BLOCK_INDEX_MAX; ++i )
    {
        // 同一个 block 的话，直接就赋值了，不需要重新加载
        if( m_kOldBlocks[i].pMapBlock != NULL && pBlockInfo->strBlockFileName.compare( m_kOldBlocks[i].strBlockFileName ) == 0 )
        {
            m_kBlocks[nIndex] = m_kOldBlocks[i];

            m_kOldBlocks[i].pMapBlock = NULL;
            m_kOldBlocks[i].strBlockFileName.clear();
            m_kOldBlocks[i].x = 0.0f;
            m_kOldBlocks[i].y = 0.0f;

            return;
        }
    }

    // 备份里面没有，只好重新加载了
    m_kBlocks[nIndex].pMapBlock = loadBlock( pBlockInfo->strBlockFileName );
    m_kBlocks[nIndex].strBlockFileName = pBlockInfo->strBlockFileName;
    m_kBlocks[nIndex].x = pBlockInfo->x;
    m_kBlocks[nIndex].y = pBlockInfo->y;

    addChild( m_kBlocks[nIndex].pMapBlock );
}

BlockInfo* TLSeamlessMap::getBlockInfo( float x, float y )
{
    // 修正坐标
    correctCoordinate( x, y );

    std::list<BlockInfo*>::iterator iter = m_listAllBlocks.begin();
    std::list<BlockInfo*>::iterator iter_end = m_listAllBlocks.end();
    for( ; iter != iter_end; ++iter )
    {
        BlockInfo* pBlockInfo = (*iter);
        if( pBlockInfo->x - m_nBlockWidth * 0.5f <= x &&
                pBlockInfo->x + m_nBlockWidth * 0.5f >= x &&
                pBlockInfo->y - m_nBlockHeight * 0.5f <= y &&
                pBlockInfo->y + m_nBlockHeight * 0.5f >= y )
        {
            return pBlockInfo;
        }
    }

    return NULL;
}

TLMapBlock* TLSeamlessMap::getMapBlock( float x, float y )
{
	correctCoordinate( x, y );

	for( int i=BLOCK_INDEX_CENTER; i < BLOCK_INDEX_MAX; ++i )
	{
		MBInfo mbInfo = m_kBlocks[i];
		if( mbInfo.pMapBlock == NULL )
			continue;

        if( mbInfo.x - m_nBlockWidth * 0.5f <= x &&
            mbInfo.x + m_nBlockWidth * 0.5f >= x &&
            mbInfo.y - m_nBlockHeight * 0.5f <= y &&
            mbInfo.y + m_nBlockHeight * 0.5f >= y )
        {
            return mbInfo.pMapBlock;
        }
	}

	return NULL;
}

void TLSeamlessMap::addBlock( const std::string& strBlockName, float x, float y )
{
    // 在修正前，先记录下来，作为当前的坐标
    m_fCurX = x;
    m_fCurY = y;

    // 修正坐标
    correctCoordinate( x, y );

    // 不能重复
    removeBlock( x, y );

    // 
    BlockInfo* pBlockInfo = new BlockInfo;
    pBlockInfo->strBlockFileName = strBlockName;
    pBlockInfo->x = x;
    pBlockInfo->y = y;
    m_listAllBlocks.push_back( pBlockInfo );

    // 更新
    updateBlock();
}

void TLSeamlessMap::removeBlock( float x, float y )
{
    // 修正坐标
    correctCoordinate( x, y );

    std::list<BlockInfo*>::iterator iter = m_listAllBlocks.begin();
    std::list<BlockInfo*>::iterator iter_end = m_listAllBlocks.end();
    for( ; iter != iter_end; ++iter )
    {
        BlockInfo* pBlockInfo = (*iter);
        if( pBlockInfo->x - m_nBlockWidth * 0.5f <= x &&
            pBlockInfo->x + m_nBlockWidth * 0.5f >= x &&
            pBlockInfo->y - m_nBlockHeight * 0.5f <= y &&
            pBlockInfo->y + m_nBlockHeight * 0.5f >= y )
        {
            m_listAllBlocks.erase( iter );

            delete pBlockInfo;

            break;
        }
    }
}

void TLSeamlessMap::correctCoordinate( float& x, float& y )
{
    int xTempX = ( (int)( x + m_nBlockWidth * 0.5f ) ) / m_nBlockWidth;
    x = xTempX * m_nBlockWidth;

    int nTempY = ( (int)( y + m_nBlockHeight * 0.5f ) ) / m_nBlockHeight;
    y = nTempY * m_nBlockHeight;
}
