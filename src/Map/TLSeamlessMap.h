#pragma once
#include <sprite_nodes/CCSprite.h>
#include <string>
#include <list>

enum BLOCK_INDEX 
{
    BLOCK_INDEX_CENTER          = 0,
    BLOCK_INDEX_LEFT,
    BLOCK_INDEX_RIGHT,
    BLOCK_INDEX_TOP,
    BLOCK_INDEX_BOTTOM,
    BLOCK_INDEX_LEFT_TOP,
    BLOCK_INDEX_LEFT_BOTTOM,
    BLOCK_INDEX_RIGHT_TOP,
    BLOCK_INDEX_RIGHT_BOTTOM,

    BLOCK_INDEX_MAX,
};

struct BlockInfo {
    std::string strBlockFileName;           // 
    float x, y;                             // position
};

USING_NS_CC;

class TLMapBlock;
class TLSeamlessMap : public CCNode
{
public:
	TLSeamlessMap( const std::string& strSeamlessMapFile, float x, float y );
    virtual ~TLSeamlessMap();

    static TLSeamlessMap* create( const std::string& strSeamlessMapFile, float x, float y );
    static bool newSeamlessMap( const std::string& strSeamlessMapFile, int nBlockRow, int nBlockCol, int nGridWidth, int nGridHeight );

    virtual bool init();

public:
    TLMapBlock* loadBlock( const std::string& strFileName );
    void updateBlock();

    BlockInfo* getBlockInfo( float x, float y );

protected:
    std::string m_strSeamlessMapFile;
    float m_fCurX, m_fCurY;

    struct MBInfo {
        TLMapBlock* pMapBlock;
        std::string strBlockFileName;           // 
        float x, y;                             // position
    };
    MBInfo m_kBlocks[BLOCK_INDEX_MAX];
    MBInfo m_kOldBlocks[BLOCK_INDEX_MAX];

public:
    void addBlock( const std::string& strBlockName, float x, float y );
    void removeBlock( float x, float y );

protected:
    void correctCoordinate( float& x, float& y );

    void updateBlock( int nIndex, BlockInfo* pBlockInfo );

protected:
    int m_nBlockRow, m_nBlockCol;               // 每个 block 的行数和列数
    int m_nGridWidth, m_nGridHeight;            // 每一个格子的高宽
    int m_nBlockWidth, m_nBlockHeight;          // 每块的高宽 m_nBlockWidth = m_nBlockCol * m_nGridWidth; m_nBlockHeight = m_nBlockRow * m_nGridHeight

    std::list<BlockInfo*> m_listAllBlocks;
};
